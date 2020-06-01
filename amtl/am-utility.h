// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013-2014, David Anderson and AlliedModders LLC
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

#ifndef _include_amtl_utility_h_
#define _include_amtl_utility_h_

#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <amtl/am-cxx.h>

#if defined(_MSC_VER)
// Mac file format warning.
#    pragma warning(disable : 4355)
#endif

namespace ke {

// Zero out a non-array pointer.
template <typename T>
static inline void
MemsetZero(T* t) {
    memset(t, 0, sizeof(*t));
}

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#define IS_ALIGNED(addr, alignment) (!(uintptr_t(addr) & ((alignment)-1)))

#if defined(__GNUC__)
#    define KE_CRITICAL_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#    define KE_CRITICAL_LIKELY(x) x
#endif

template <typename T>
struct cast_to_pointer {
    static void* cast(const T& t) {
        return reinterpret_cast<void*>((uintptr_t)t);
    }
};

template <typename T>
struct cast_to_pointer<T*> {
    static const void* cast(const T* ptr) {
        return ptr;
    }
    static void* cast(T* ptr) {
        return ptr;
    }
};

} // namespace ke

#endif // _include_amtl_utility_h_
