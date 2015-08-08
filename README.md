The AlliedModders Template Library is a small set of C++ headers. It aims to be lightweight,
supporting a minimalistic set of abstractions and data structures useful for high-performance C++
systems code. It is the spiritual succesor to the SourceHook template library.

AMBuild currently requires C++11 support. The minimum supported compiler versions are:
 - Microsoft Visual Studio 2010 or higher.
 - GNU GCC 4.5 or higher.
 - Clang 3.0 or higher.

Specifically, it requires `nullptr`, `>>` support in templates, and r-value references v2.0,
(N2844). The older, deprecated `c++98` branch of AMTL does not require C++11.

# Goals

* Performance Over Generality. Rather than accomodate every use case, AMTL strives to make
  performance explicit, and does not include features which are dangerous for performance, or
  features that make malloc() failure detection more difficult.

  For example, while STL provides copy constructors for most containers, AMTL does not, as these
  operations are unsafe for fallible malloc(), and can cause large unintentional performance hits.

  Another example is boost::shared\_ptr, which by default is atomic and threadsafe, and
  preprocessor macros can be used to eliminate the thread safety option. AMTL opts to use distinct
  types for threadsafe and non-threadsafe reference counting, as the cost of atomic refcounting
  is higher.

* Safety. Most code in AlliedModders does not take advantage of RAII, and does not account for
  fallible malloc. While in general, infallible malloc is probably okay for game servers, the lack
  of RAII leads to very complicated and bloated pointer management. A major goal of AMTL is to
  introduce safer pointer management into its codebases, and to allow certain components to safely
  handle malloc failures when desired.

* No linkage. Everything is contained within headers so that tools don't need to link additional
  C++ files. This could be a drawback, for example, it could increase build time or codesize
  bloat. We may evaluate this decision in the future, but for now, it fits into our "lightweight"
  theme of usability.

  (On some platforms, system libraries may be needed for certain headers.)

* No RTTI or EH. There are a few reasons for this:
  + Both features require linkage to libstdc++. On Linux, this can cause problems with binary
    distribution.

  + When using 32-bit Visual C++, try blocks are not zero-cost, as they are implemented using SEH.

* Fallible malloc. In many cases it's not desirable to terminate the program or throw an
  exception when an allocation fails (for example, resizing a large vector). Thus, functions
  like ke::Vector::append (analagous to std::vector::push\_back) will usually return bool. Whether
  or not these functions return (i.e., abort the program or fail safely) can be specified in an
  allocation policy.

* Room to grow. These templates are intended to be a baseline of features needed for various
  AlliedModders projects. They can and will grow as new features are needed, hopefully keeping the
  above principles in mind.

# Overview

All classes and functions reside in a "ke" namespace, and all macros are prefixed with "KE\_".

### Platform Utilities (am-utilities.h)

This provides some common helper functions pretty much lumped together:
* AutoPtr<T>, RAII for calling delete on a pointer.
* AutoArray<T>, RAII for calling delete[] on a pointer.
* Detectors for add or multiply overflow, useful for computing allocation sizes.
* Fast Log2/Floor2 functions (also known as bitscan or count zeroes).
* Integer alignment (useful for allocators or code generators).
* KE\_DELETE and KE\_OVERRIDE which map to delete and override keywords if C++11 is available.

### Moveable (am-moveable.h)

AMTL used to emulate C++ r-value semantics. Now it requires native support. It
provides a replacement for std::move as ke::Move, and std::forward as ke::Forward.

### Thread Utilities (am-thread-utils.h)

Thread utils provides access to common sychronization primitives: mutexes, condition variables,
and threads. It also provides RAII objects for acquiring and releasing locks. More information is
available at the top of am-thread-utils.h.

In addition, there is a `ThreadLocal` container available in am-threadlocal.h. By default it acts
as a per-thread storage container, using operating system primitives. The container can hold a
single value that is castable to `uintptr_t`. It can also be used in single-threaded systems via
`KE_SINGLE_THREADED`, at which point there is no TLS overhead. This is useful for systems where
threading is optional.

### Reference Counting (am-refcounting.h)

Two classes are available to help other classes implement reference counting. They are Refcounted
and RefcountedThreadsafe. Both must be inherited recursively, for example:

    class MyObject : public Refcounted<MyObject>
  
The object is instantiated with a reference count of 0, meaning it is illegal to call Release()
without first calling AddRef.

A few RAII objects are provided for automatically retaining and releasing references:

* Ref<T>. Adds one reference upon construction and releases one reference on destruction.
* PassRef<T>. Same as Ref<T>, except, when passed into a Ref<T> or into a PassRef<T>, the contained
  reference is NULL'd, rather than performing an unnecessary extra AddRef()+Release(). PassRef<T>
  is intended only for return values.
* AdoptRef(T). If an object is returned with an implicit AddRef, not using Ref or PassRef, then
  it is necessary to AdoptRef() when assigning into a Ref or PassRef.

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

### Vector (am-vector.h)

ke::Vector is roughly analagous to std::vector. A few method names are changed:
* size() is length()
* push\_back() is append() and returns boolean.
* The underlying buffer of a vector can be "taken", which is sometimes useful for transferring
  ownership of data.

Vector does not have an iterator type. It should be iterated through normal sequential indexing.

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

### Functions and Lambdas (am-function.h)

AMTL provides two replacements for std::function. The first, `Lambda`, serves roughly the same
purpose and uses similar mechanics. It creates a copy of the underlying callable object (inlining
the callable if it is small enough), allowing the `Lambda` to be safely copied and used outside
its originating scope. Like all AMTL containers, `ke::Lambda` supports move semantics.

`Lambda` is not always ideal since it does have to deep-copy the callable object. AMTL provides
an analog to `std::ref` as `ke::byref`. This simply wraps an l-value reference into an object
that appears to be an r-value of the same type.

### HashMap (am-hashmap.h)

ke::HashMap is a key -> value map built upon HashTable. This saves you from having to implement all
of a Policy and provides some more generic accessors. However it is still necessary to implement a
hash and comparison function.

### HashSet (am-hashset.h)

ke::HashSet is a HashMap that only stores keys. Like HashMap, you must still implement a policy
with comparison and hash functions.

### InlineList (am-inlinelist.h)

InlineList is a doubly-linked list that does use any calls to malloc(), with the caveat that list
members can be members in at most one inline list at a time. Objects must recursively inherit from
InlineListNode, which ensures that the object has prev/next pointers. Example:

    class MyObject : public InlineListNode<MyObject>
    { };
    InlineList<MyObject> list;

InlineList is ideal for situations where objects have already been allocated and will be a member
of at most one list at any time. If this is the case, the per-node malloc/free of std::list can be
completely avoided.

An InlineList never claims ownership over its pointers. Its destructor is a no-op and it makes no
attempt to destroy members of its list (though in debug builds it will unlink members in its
destructor for assertion purposes).

### LinkedList (am-linkedlist.h)

LinkedList is a doubly-linked list that requires a malloc() for each entry. It is provided for
backward compatibility, and it should be avoided in general, except in situations where it is
absolutely necessary and allocation performance is not a factor. Otherwise, use InlineList or
Vector.

### Strings (am-string.h)

The String API is still rather in flux. Currently, a simple std::string-like class is provided
called AString (short for ASCII String).

### FixedArray (am-fixedarray.h)

FixedArray&lt;T, AP&gt; is a stripped-down version of Vector that is constructed with a fixed
length. The length is passed via the constructor; a compile-time length variant is planned.

# Replacing SourceHook Includes

AMTL is a spiritual successor to the SourceHook template library, used in many AlliedModders
projects. AMTL is intended to deprecate it.

### CVector (sh\_vector.h)

CVector can be replaced with ke::Vector. ke::Vector does not provide an iterator (yet). A few
method names have been changed, as CVector more closely models std::vector, however the classes are
pretty similar. Any missing methods could be added quite easily.

### String (sh\_string.h)

String can be replaced with ke::AString, however some methods like trim() are not available yet.
One notable difference is that AString caches the length of the string, whereas SH::String does
not. That means AString is a larger data structure (by 4 bytes, on 32-bit), but is not prone to
O(n^2) operations like String. This more closely matches the functionality of std::string.

### List (sh\_list.h)

List can be replaced with ke::LinkedList. A few methods have changed, similar to Vector. For
example, push\_back() becomes append() and push\_front() becomes prepend(), and both return
boolean.

Consider using ke::InlineList if your list contains already-allocated objects, and those objects
are members of at most one list. LinkedList is inefficient as it will call malloc() and free() for
each node.

The implementation of SourceHook::List and ke::LinkedList are basically identical underneath the
hood, however, copy construction and assignment are disallowed in the AMTL variant.

### THash<K, V> (sh\_tinyhash.h)

TinyHash can be replaced with ke::HashMap, with minor changes. Comparators must be implemented as
equality operators on the key type, and the hash function is specified as a template parameter.

TinyHash is disadvantageous for a number of reasons. It uses separate chaining, and chains are
built on top of SourceHook::List, so it will use more memory on average. Inserts are also more
expensive as a call to malloc() is almost guaranteed. Retrieval is more expensive as well, since it
requires more indirection and has worse cache locality. It also has no ability to resize table
downward when its density is very low.

### CStack (sh\_stack.h)

SourceHook::CStack is a stable-pointer stack. That is, if an element has been pushed, the address
the element is stored at is guaranteed not to change unless the element is removed. This can be an
important requirement in cases that involve re-entrancy. If you are relying on this feature of
CStack, there is no replacement in AMTL. If your CStack entries are LIFO, and tied to local
variables, you might consider simply linking objects together using RAII and avoiding CStack
entirely (though CStack will have better cache performance if you need to walk the list of entries).

If you don't need stable pointers, CStack can be replaced with ke::Vector.

# License

All headers are licensed under a 3-clause BSD license.
