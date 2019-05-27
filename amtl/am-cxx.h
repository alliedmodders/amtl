// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2014, David Anderson and AlliedModders LLC
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
#ifndef _include_amtl_cxx_support_h_
#define _include_amtl_cxx_support_h_

#if defined(__clang__)
#    if !(defined(__clang_major__) && defined(__clang_minor__))
#        define KE_CLANG_MAJOR 1
#        define KE_CLANG_MINOR __GNUC_MINOR__
#    else
#        if defined(__apple_build_version__) && clang_major__ > 3
// 4.0 => 3.1, 4.1 => 3.2
#            if __clang_major__ == 4
#                define KE_CLANG_MAJOR 3
#                if __clang_minor__ == 0
#                    define KE_CLANG_MINOR 1
#                else
#                    define KE_CLANG_MINOR 2
#                endif
// 5.0 => 3.3, 5.1 => 3.4
#            elif __clang_major__ == 5
#                define KE_CLANG_MAJOR 3
#                if __clang_minor__ == 0
#                    define KE_CLANG_MINOR 3
#                else
#                    define KE_CLANG_MINOR 4
#                endif
#            elif __clang_major__ == 6
#                define KE_CLANG_MAJOR 3
#                define KE_CLANG_MINOR 5
#            endif
#        endif
#        if !defined(KE_CLANG_MAJOR)
#            define KE_CLANG_MAJOR __clang_major__
#        endif
#        if !defined(KE_CLANG_MINOR)
#            define KE_CLANG_MINOR __clang_minor__
#        endif
#    endif

// Done with horrible clang version detection.
#    define KE_CLANG_AT_LEAST(x, y) \
        ((__clang_major__ > (x)) || (__clang_major__ == x && __clang_minor__ >= y))

#    if !KE_CLANG_AT_LEAST(3, 4)
#        error "AMTL requires clang 3.4 or higher"
#    endif

#elif defined(__GNUC__)
#    define KE_GCC_AT_LEAST(x, y) ((__GNUC__ > (x)) || (__GNUC__ == x && __GNUC_MINOR__ >= y))

#    if !KE_GCC_AT_LEAST(4, 9)
#        error "AMTL requires GCC 4.9 or higher"
#    endif

#elif defined(_MSC_VER)
#    if _MSC_VER < 1900
#        error "AMTL requires Microsoft Visual Studio 2015 or higher"
#    endif
#else
#    error "Unrecognized compiler."
#endif

// Done with compiler feature detection.

#if defined(_MSC_VER)
// This feature has been around for long enough that we shouldn't have to
// warn for it.
#    pragma warning(disable : 4351)
#endif

#if defined(_MSC_VER)
#    define KE_LINKONCE(x) __declspec(selectany) x
#else
#    define KE_LINKONCE(x) x __attribute__((weak))
#endif

#endif // _include_amtl_cxx_support_h_
