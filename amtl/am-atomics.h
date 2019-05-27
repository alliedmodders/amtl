// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2017, David Anderson and AlliedModders LLC
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

#ifndef _include_amtl_atomics_h_
#define _include_amtl_atomics_h_

#if defined(KE_ABSOLUTELY_NO_STL)
#    include <amtl/am-atomics-no-stl.h>
#else
#    include <amtl/am-atomics-stl.h>
#endif

#if defined(KE_CXX_MSVC)
extern "C" {
void* __cdecl _InterlockedCompareExchangePointer(void* volatile* Destination, void* Exchange,
                                                 void* Comparand);
} // extern "C"
#    pragma intrinsic(_InterlockedCompareExchangePointer)
#endif

#if defined(KE_CXX_LIKE_GCC)
#    if defined(i386) || defined(__x86_64__)
#        if defined(__clang__)
static inline void
YieldProcessor()
{
    asm("pause");
}
#        else
#            if KE_GCC_AT_LEAST(4, 7)
#                define YieldProcessor() __builtin_ia32_pause()
#            else
static inline void
YieldProcessor() {
    asm("pause");
}
#            endif
#        endif
#    else
#        define YieldProcessor()
#    endif
#elif defined(_MSC_VER)
#    if !defined(YieldProcessor)
#        define YieldProcessor _mm_pause
#    endif
#endif

namespace ke {

#if defined(KE_CXX_MSVC)
static inline void*
CompareAndSwapPtr(void* volatile* Destination, void* Exchange, void* Comparand) {
    return _InterlockedCompareExchangePointer(Destination, Exchange, Comparand);
}
#else
static inline void*
CompareAndSwapPtr(void* volatile* dest, void* newval, void* oldval)
{
    return __sync_val_compare_and_swap(dest, oldval, newval);
}
#endif

} // namespace ke

#endif // _include_amtl_atomics_h_
