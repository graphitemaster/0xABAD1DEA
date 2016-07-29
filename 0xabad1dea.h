#ifndef _0xABAD1DEA
#define _0xABAD1DEA

#include <new>
#include <string.h>

// Change this to which ever locking primitive you use if need be
#ifndef NOTHREADSAFE
#include <pthread.h>
struct StaticLock {
  StaticLock() { pthread_mutex_init(&m_mutex, nullptr); }
  ~StaticLock() { pthread_mutex_destroy(&m_mutex); }
  void lock() { pthread_mutex_lock(&m_mutex); }
  void unlock() { pthread_mutex_unlock(&m_mutex); }
private:
  pthread_mutex_t m_mutex;
};
#else
struct StaticLock {
  void lock() { }
  void unlock() { }
};
#endif

struct StaticNode;
static StaticNode *gHead, *gTail;
static StaticLock gLock; // lock protecting linked list

struct StaticLockGuard {
  StaticLockGuard(StaticLock* lock) : m_lock(lock) { m_lock->lock(); }
  ~StaticLockGuard() { m_lock->unlock(); }
private:
  StaticLock* m_lock;
};

template <typename T>
struct StaticTypeCarry { typedef T type; };

struct StaticNode {
  template <typename T>
  StaticNode(const char *name, StaticTypeCarry<T>) : m_name(name), m_next(nullptr), m_prev(nullptr), m_ctor(ctor_<T>), m_dtor(dtor_<T>) {
    StaticLockGuard locked(&gLock);
    if (!gHead) gHead = this;
    if (gTail) m_prev = gTail, m_prev->m_next = this;
    gTail = this;
  }
  void construct() { m_ctor(this + 1); }
  void destruct() { m_dtor(this + 1); }
private:
  template <typename T>
  static void ctor_(void *obj) { new ((T *)obj) T; }
  template <typename T>
  static void dtor_(void *obj) { ((T *)obj)->~T(); }
  friend struct StaticGlobals;
  const char *m_name;
  StaticNode *m_next, *m_prev;
  void (*m_ctor)(void *), (*m_dtor)(void *);
};

template <typename T>
struct StaticGlobal {
  StaticGlobal(const char *name) : node(name, StaticTypeCarry<T>()) { }
  T &operator*() { return (T &)data; }
  T *operator->() { return (T *)data; }
  const T &operator*() const { return (const T&)data; }
  const T *operator->() const { return (const T*)data; }
private:
  StaticNode node;
  alignas(alignof(T)) unsigned char data[sizeof(T)];
};

struct StaticGlobals {
  static void initialize() {
    StaticLockGuard locked(&gLock);
    for (StaticNode *node = gHead; node; node = node->m_next)
      node->construct();
  }
  static void deinitialize() {
    StaticLockGuard locked(&gLock);
    for (StaticNode *node = gTail; node; node = node->m_prev)
      node->destruct();
  }
  static StaticNode* find(const char *name) {
    StaticLockGuard locked(&gLock);
    for (StaticNode *node = gHead; node; node = node->m_next)
      if (!strcmp(node->m_name, name))
        return node;
    return nullptr;
  }
  static void remove(StaticNode *node) {
    StaticLockGuard locked(&gLock);
    if (node->m_next) node->m_next->m_prev = node->m_prev;
    if (node->m_prev) node->m_prev->m_next = node->m_next;
    if (gHead == node) gHead = node->m_next;
    if (gTail == node) gTail = node->m_prev;
  }
};

#endif
