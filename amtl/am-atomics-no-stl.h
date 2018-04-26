// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013, David Anderson and AlliedModders LLC
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//  * Neither the name of AlliedModders LLC nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef _include_amtl_atomics_no_stl_h_
#define _include_amtl_atomics_no_stl_h_

#include <amtl/am-cxx.h>
#include <amtl/am-platform.h>
#include <stddef.h>
#include <stdint.h>

namespace ke {

#if defined(KE_CXX_MSVC)
extern "C" {
  long __cdecl _InterlockedIncrement(long volatile* dest);
  long __cdecl _InterlockedDecrement(long volatile* dest);
  long long __cdecl _InterlockedIncrement64(long long volatile* dest);
  long long __cdecl _InterlockedDecrement64(long long volatile* dest);
}
# pragma intrinsic(_InterlockedIncrement)
# pragma intrinsic(_InterlockedDecrement)
# if defined(_WIN64)
#  pragma intrinsic(_InterlockedIncrement64)
#  pragma intrinsic(_InterlockedDecrement64)
# endif
#endif

template <size_t Width>
struct AtomicOps;

template <>
struct AtomicOps<4>
{
#if defined(KE_CXX_MSVC)
  typedef volatile long Type;

  static Type Increment(Type* ptr) {
    return _InterlockedIncrement(ptr);
  }
  static Type Decrement(Type* ptr) {
    return _InterlockedDecrement(ptr);
  };
#elif defined(KE_CXX_LIKE_GCC)
  typedef volatile int Type;

  // x86/x64 notes: When using GCC < 4.8, this will compile to a spinlock.
  // On 4.8+, or when using Clang, we'll get the more optimal "lock addl"
  // variant.
  static Type Increment(Type* ptr) {
    return __sync_add_and_fetch(ptr, 1);
  }
  static Type Decrement(Type* ptr) {
    return __sync_sub_and_fetch(ptr, 1);
  }
#endif
};

template <>
struct AtomicOps<8>
{
#if defined(KE_CXX_MSVC)
  typedef volatile long long Type;

  static Type Increment(Type* ptr) {
    return _InterlockedIncrement64(ptr);
  }
  static Type Decrement(Type* ptr) {
    return _InterlockedDecrement64(ptr);
  };
#elif defined(KE_CXX_LIKE_GCC)
  typedef volatile int64_t Type;

  // x86/x64 notes: When using GCC < 4.8, this will compile to a spinlock.
  // On 4.8+, or when using Clang, we'll get the more optimal "lock addl"
  // variant.
  static Type Increment(Type* ptr) {
    return __sync_add_and_fetch(ptr, 1);
  }
  static Type Decrement(Type* ptr) {
    return __sync_sub_and_fetch(ptr, 1);
  }
#endif
};

class AtomicRefcount
{
  typedef AtomicOps<sizeof(uintptr_t)> Ops;

 public:
  explicit AtomicRefcount(uintptr_t value)
   : value_(value)
  {
  }

  void increment() {
    Ops::Increment(&value_);
  }

  // Return false if all references are gone.
  bool decrement() {
    return Ops::Decrement(&value_) != 0;
  }

 private:
  Ops::Type value_;
};

} // namespace ke

#endif // _include_amtl_atomics_no_stl_h_
