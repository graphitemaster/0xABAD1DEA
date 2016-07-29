# 0xABAD1DEA

This little header provides an alternative way to construct and destruct static
global objects.

# How to use
Add `0xabad1dea.cpp` and `0xabad1dea.h` to your project.

# Examples

## Trivial
```c++
struct A {
  A() { printf("A::A()\n"); }
  ~A() { printf("A::~A()\n"); }
};
StaticGlobal<A> gMyA0("MyA0"), gMyA1("MyA1"); // uninitialized

int main() {
  StaticGlobals::initialize(); // calls the constructors objects are now initialized

  // typically best to use atexit(StaticGlobals::deinitialize)
  StaticGlobals::deinitialize(); // calls the destructors
}
```

## Calling individuals
```c++
int main() {
  StaticNode *node = StaticGlobals::find("MyA1");
  if (node) node->construct(); // manually initialize MyA1
  // ...
  if (node) node->destruct(); // manually destruct MyA1
}
```

## Removing individuals
```c++
int main() {
 StaticNode *node = StaticGlobals::find("MyA1");
 if (node) StaticGlobals::remove(node); // remove MyA1 from initialization and destruction
}
```

## Real world example
Have a memory allocator which needs to be allocated before all other static globals,
you could lazily construct it with a singleton pattern or you could use something like:
```c++
StaticGlobal<Allocator> gAllocator("Allocator");
/// ...
int main() {
  atexit(StaticGlobals::deinitialize); // be sure to call destructors of statics at exit
  StaticNode *node = StaticGlobals::find("Allocator");
  if (node) {
    node->initialize(); // call the constructor
    StaticGlobals::remove(node); // remove from the statics list
  }
  StaticGlobals::initialize(); // initialize all other statics

  // ... typical code

  if (node)
    node->deinitialize(); // deinitialize the allocator
}
```

Of course the other statics may depend on the allocator to exist (e.g free memory)
so we cannot deinitialize in main, what we could do is something like this which
uses atexit to register a lambda function which destroys the statics before the
other atexit handler for all other static global objects runs.
```c++
StaticNode *node;
int main() {
  node = StaticGlobals::find("Allocator");
  if (node) {
    node->initialize(); // call the constructor
    atexit([](){ node->deinitialize(); }); // this atexit will be called first
    StaticGlobals::remove(node); // remove from the statics list
  }
  StaticGlobals::initialize(); // initialize all other statics
  atexit(StaticGlobals::deinitialize); // be sure to call destructors of statics at exit

  // normal code
}
```

# How it works
Using an intrusive linked list to thread a doubly linked list of each global
using static storage and then iterating that linked list to actually call
placement new on uninitialized storage to construct the static objects.
The same is done for destruction except the tail end of the linked list is
used so things get destroyed in reverse order as they were constructed.

# Thread safety
Currently uses a global lock via `pthread.h` but it's about 10 lines of code
and can be replaced with which ever locking primitive your API may have.

# License
```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
```
