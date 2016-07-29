#ifndef _0xABAD1DEA
#define _0xABAD1DEA

#include <new>

template <typename T>
struct StaticTypeCarry { typedef T type; };

struct StaticNode {
  template <typename T>
  StaticNode(const char *name, StaticTypeCarry<T>) : m_name(name), m_next(nullptr), m_prev(nullptr), m_ctor(ctor_<T>), m_dtor(dtor_<T>) { init(); }
  void construct() { m_ctor(this + 1); }
  void destruct() { m_dtor(this + 1); }
private:
  void init();
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
  static void initialize();
  static void deinitialize();
  static StaticNode* find(const char *name);
  static void remove(StaticNode *node);
};

#endif
