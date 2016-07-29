#include "0xabad1dea.h"
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

void StaticNode::init() {
  StaticLockGuard locked(&gLock);
  if (!gHead) gHead = this;
  if (gTail) m_prev = gTail, m_prev->m_next = this;
  gTail = this;
}

void StaticGlobals::initialize() {
  StaticLockGuard locked(&gLock);
  for (StaticNode *node = gHead; node; node = node->m_next)
    node->construct();
}

void StaticGlobals::deinitialize() {
  StaticLockGuard locked(&gLock);
  for (StaticNode *node = gTail; node; node = node->m_prev)
    node->destruct();
}

StaticNode* StaticGlobals::find(const char *name) {
  StaticLockGuard locked(&gLock);
  for (StaticNode *node = gHead; node; node = node->m_next)
    if (!strcmp(node->m_name, name))
      return node;
  return nullptr;
}

void StaticGlobals::remove(StaticNode *node) {
  StaticLockGuard locked(&gLock);
  if (node->m_next) node->m_next->m_prev = node->m_prev;
  if (node->m_prev) node->m_prev->m_next = node->m_next;
  if (gHead == node) gHead = node->m_next;
  if (gTail == node) gTail = node->m_prev;
}
