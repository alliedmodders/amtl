The AlliedModders Template Library is a small set of C++ headers. It aims to be lightweight,
supporting a minimalistic set of abstractions and data structures useful for high-performance C++
systems code. It is the spiritual successor to the SourceHook template library.

### Requirements:
 - C++17
 - Microsoft Visual Studio 2017 or higher.
 - Clang 5.0 or higher.
 - GNU GCC 8.0 or higher.

# Goals

* Performance Over Generality. Rather than accomodate every use case, AMTL strives to make
  performance explicit.

* Safety. Most code in AlliedModders does not take advantage of RAII due to predating C++11,
  and AMTL provides helpers to gradually modernize codebases.

* No linkage. Everything is contained within headers so that tools don't need to link additional
  C++ files (with the exception of arg-parser).

* No RTTI or EH. There are a few reasons for this:
  + RTTI greatly increases binary size and potentially linkage issues.

  + When using 32-bit Visual C++, try blocks are not zero-cost, as they are implemented using SEH.

# Overview

All classes and functions reside in a "ke" namespace, and all macros are prefixed with "KE\_".

### Platform Utilities (am-utilities.h)

This provides some common helper functions pretty much lumped together:
* Detectors for add or multiply overflow, useful for computing allocation sizes.
* Fast Log2/Floor2 functions (also known as bitscan or count zeroes).
* Integer alignment (useful for allocators or code generators).

### Reference Counting (am-refcounting.h)

Two classes are available to help other classes implement reference counting. They are Refcounted
and RefcountedThreadsafe. Both must be inherited recursively, for example:

    class MyObject : public Refcounted<MyObject>
  
The object is instantiated with a reference count of 0, meaning it is illegal to call Release()
without first calling AddRef.

A few RAII objects are provided for automatically retaining and releasing references:

* RefPtr<T>. Adds one reference upon construction and releases one reference on destruction.
* AdoptRef(T). If an object is returned with an implicit AddRef, not using RefPtr, then
  it is necessary to AdoptRef() when assigning into a RefPtr.

RefcountedThreadsafe provides the same accessors as Refcounted. However, the reference counting is
performed using atomic operations, so that two threads racing do not accidentally result in a wrong
reference count (since a read + write is not normally atomic). This does not make the actual object
threadsafe, rather, it makes it safe for multiple threads to mutate the reference count.

### Allocator Policies (am-allocator-policies.h)

ke::HashTable and ke::Vector are both parameterized on an "AllocPolicy". It is accessed whenever a
malloc(), free(), size overflow, or out-of-memory operation occurs. The default functionality of
SystemAllocatorPolicy is to abort the process on any failure, which can be changed by implementing
a new policy. As HashTable and Vector inherit from the AllocPolicy, it is possible to use custom
allocators without accessing static variables.

### STL Helpers

These are in `am-vector.h`, `am-deque.h`, etc. They contain helpers for working with STL data
structures, for example, by providing alternatives to `pop_back()` that can return the popped
value.

### HashTable (am-hashtable.h)

ke::HashTable is an open-addressing, linear-probing hash table. It is a bit more powerful than a
generic key -> value map as it is templated on a "policy", rather than keys and values. A policy
specifies what entries in the table look like, and how they are searched. Policies must have three
members:

* Payload - the type that will be stored for each entry in the hash table.
* static bool matches(const T &t, Payload &p); - Must return true if |t| is equal to the entry at |p|.
* static uin32\_t hash(const T &t); - Must compute a hash for |t|.

The typename T in the policy applies for every key type that will be used in the hash table. For
example, a hash table might store string keys, but have faster lookup using std::string versus a
C-string, or even faster lookup for a string type that has its hash precomputed.

All key types must effectively compute the same hash; i.e., it would not be legal for Payload to
match both |int| and |std::string| but compute different hashes.

HashTable returns either a Result or Insert object when performing lookups, depending on the type
of lookup. A Result object is either found or not found. An Insert object, however, caches
information about where it found the object, so that it doesn't have to repeat the hashing and
lookup process for the subsequent add() call. Examples on how to use this are in am-hashmap.h, and
the exact rules on how Result and Insert objects can be used are in am-hashtable.h.

HashTables can be iterated. If the table is mutated during iteration outside of the interface
provided by the iterator, then the iterator may become corrupted. This is acceptable as long as the
iterator is abandoned. However, the table must stay alive for as long as the iterator is active.

### HashMap (am-hashmap.h)

ke::HashMap is a key -> value map built upon HashTable. This saves you from having to implement all
of a Policy and provides some more generic accessors. However it is still necessary to implement a
hash and comparison function.

Consider using std::unordered\_map or std::unordered\_set if your use case
doesn't benefit from open addressing.

### HashSet (am-hashset.h)

ke::HashSet is a HashMap that only stores keys. Like HashMap, you must still implement a policy
with comparison and hash functions.

### InlineList (am-inlinelist.h)

InlineList is a doubly-linked list that does not use any calls to malloc(), with the caveat that
list members can be members in at most one inline list at a time. Objects must recursively inherit
from InlineListNode, which ensures that the object has prev/next pointers. Example:

    class MyObject : public InlineListNode<MyObject>
    { };
    InlineList<MyObject> list;

InlineList is ideal for situations where objects have already been allocated and will be a member
of at most one list at any time. If this is the case, the per-node malloc/free of std::list can be
completely avoided.

An InlineList never claims ownership over its pointers. Its destructor is a no-op and it makes no
attempt to destroy members of its list (though in debug builds it will unlink members in its
destructor for assertion purposes).

### FixedArray (am-fixedarray.h)

FixedArray&lt;T, AP&gt; is a stripped-down version of Vector that is constructed with a fixed
length. The length is passed via the constructor; a compile-time length variant is planned.

### Flags (am-flags.h)

`Flags<T>` is a container class for a scoped enum type (that is, an enum declared as `enum class`).
It is meant to be used in addition to (or as a replacement of) `KE_DEFINE_ENUM_OPERATORS`. It is
designed for enums where each member is a bit or bitmask. Flags provides type-safe bitwise
operations, and most importantly, an implicit cast-to-bool operator which is not allowed for
normal C++ enumerations.

### args::Parser (experimental/am-argparser.h)

This is currently experimental. It is a command-line argv parser similar to Python's argparse
library. It is designed to handle a few basic cases and will be expanded as needed. Examples
of how to use it are in test-argparse.cpp.

### Link Helpers

- `compat/stdcxx.cpp` can be included on Linux builds to work around highly idiosyncratic libstdc++
  behavior when distributing libstdc++ based binaries.

# Replacing SourceHook Includes

AMTL is a spiritual successor to the SourceHook template library, used in many AlliedModders
projects. AMTL is intended to deprecate it.

### CVector (sh\_vector.h)

CVector can be trivially replaced with std::vector.

### String (sh\_string.h)

String can be replaced with std::string.

### List (sh\_list.h)

List can be replaced with std::list.

Consider using ke::InlineList if your list contains already-allocated objects, and those objects
are members of at most one list. std::list is inefficient as it will call malloc() and free() for
each node.

### THash<K, V> (sh\_tinyhash.h)

TinyHash can be replaced with ke::HashMap, with minor changes. Comparators must be implemented as
equality operators on the key type, and the hash function is specified as a template parameter.

TinyHash is disadvantageous for a number of reasons. It uses separate chaining, and chains are
built on top of SourceHook::List, so it will use more memory on average. Inserts are also more
expensive as a call to malloc() is almost guaranteed. Retrieval is more expensive as well, since it
requires more indirection and has worse cache locality. It also has no ability to resize table
downward when its density is very low.

You can also consider std::unordered\_map if you don't need open addressing.

### CStack (sh\_stack.h)

SourceHook::CStack is a stable-pointer stack. That is, if an element has been pushed, the address
the element is stored at is guaranteed not to change unless the element is removed. This can be an
important requirement in cases that involve re-entrancy. If you are relying on this feature of
CStack, there is no replacement in AMTL. If your CStack entries are LIFO, and tied to local
variables, you might consider simply linking objects together using RAII and avoiding CStack
entirely (though CStack will have better cache performance if you need to walk the list of entries).

If you don't need stable pointers, CStack can be replaced with std::vector.

# Compile-Time Options

- `KE_ALLOW_STD_CXX` - In some cases, STL is implemented using compiler intrinsics, and therefore
  certain features are not possible to implement without using STL. An example of this would be
  std::underlying\_type. AMTL will try to use an intrinsic if available, but if for some reason
  it does not work, you can enable the STL version with this macro.

# License

All headers are licensed under a 3-clause BSD license.
