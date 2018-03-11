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
# if !(defined(__clang_major__) && defined(__clang_minor__))
#  define KE_CLANG_MAJOR 1
#  define KE_CLANG_MINOR __GNUC_MINOR__
# else
#  if defined(__apple_build_version__) && clang_major__ > 3
    // 4.0 => 3.1, 4.1 => 3.2
#   if __clang_major__ == 4
#    define KE_CLANG_MAJOR 3
#    if __clang_minor__ == 0
#     define KE_CLANG_MINOR 1
#    else
#     define KE_CLANG_MINOR 2
#    endif
    // 5.0 => 3.3, 5.1 => 3.4
#   elif __clang_major__ == 5
#    define KE_CLANG_MAJOR 3
#    if __clang_minor__ == 0
#     define KE_CLANG_MINOR 3
#    else
#     define KE_CLANG_MINOR 4
#    endif
#   elif __clang_major__ == 6
#    define KE_CLANG_MAJOR 3
#    define KE_CLANG_MINOR 5
#   endif
#  endif
#  if !defined(KE_CLANG_MAJOR)
#   define KE_CLANG_MAJOR __clang_major__
#  endif
#  if !defined(KE_CLANG_MINOR)
#   define KE_CLANG_MINOR __clang_minor__
#  endif
# endif

// Done with horrible clang version detection.
# define KE_CLANG_AT_LEAST(x, y) \
   ((__clang_major__ > (x)) || (__clang_major__ == x && __clang_minor__ >= y))

# if KE_CLANG_AT_LEAST(2, 9)
#  define KE_CXX_HAS_RVAL_REFS 30
#  define KE_CXX_HAS_DELETE
#  define KE_CXX_HAS_STATIC_ASSERT
#  define KE_CXX_HAS_DOUBLE_GT
#  define KE_CXX_HAS_ENUM_CLASS
# endif
# if KE_CLANG_AT_LEAST(3, 0)
#  define KE_CXX_HAS_OVERRIDE
#  define KE_CXX_HAS_EXPLICIT_BOOL
#  define KE_CXX_HAS_NULLPTR
#  define KE_CXX_HAS_NOEXCEPT
# endif
# if KE_CLANG_AT_LEAST(3, 1)
#  define KE_CXX_HAS_CONSTEXPR
# endif
# if KE_CLANG_AT_LEAST(3, 4)
#  define KE_CXX_HAS_GENERIC_LAMBDA_CAPTURES
# endif

#elif defined(__GNUC__)
# define KE_GCC_AT_LEAST(x, y) ((__GNUC__ > (x)) || (__GNUC__ == x && __GNUC_MINOR__ >= y))

# if KE_GCC_AT_LEAST(4, 3)
#  define KE_CXX_HAS_RVAL_REFS 10
#  define KE_CXX_HAS_STATIC_ASSERT
#  define KE_CXX_HAS_DOUBLE_GT
# endif
# if KE_GCC_AT_LEAST(4, 4)
#  define KE_CXX_HAS_DELETE
#  define KE_CXX_HAS_ENUM_CLASS
# endif
# if KE_GCC_AT_LEAST(4, 5)
#  define KE_CXX_HAS_EXPLICIT_BOOL
#  undef KE_CXX_HAS_RVAL_REFS
#  define KE_CXX_HAS_RVAL_REFS 21
# endif
# if KE_GCC_AT_LEAST(4, 6)
#  define KE_CXX_HAS_NULLPTR
#  define KE_CXX_HAS_NOEXCEPT
#  define KE_CXX_HAS_CONSTEXPR
#  undef KE_CXX_HAS_RVAL_REFS
#  define KE_CXX_HAS_RVAL_REFS 30
# endif
# if KE_GCC_AT_LEAST(4, 7)
#  define KE_CXX_HAS_OVERRIDE
# endif
# if KE_GCC_AT_LEAST(4, 9)
#  define KE_CXX_HAS_GENERIC_LAMBDA_CAPTURES
# endif

#elif defined(_MSC_VER)
# if _MSC_VER >= 1600
#  define KE_CXX_HAS_RVAL_REFS 20
#  define KE_CXX_HAS_STATIC_ASSERT
#  define KE_CXX_HAS_DOUBLE_GT
#  define KE_CXX_HAS_NULLPTR
# endif
# if _MSC_VER >= 1700
#  undef KE_CXX_HAS_RVAL_REFS
#  define KE_CXX_HAS_RVAL_REFS 21
#  define KE_CXX_HAS_OVERRIDE
#  define KE_CXX_HAS_ENUM_CLASS
# endif
# if _MSC_VER >= 1800
#  define KE_CXX_HAS_DELETE
#  define KE_CXX_HAS_EXPLICIT_BOOL
# endif
# if _MSC_VER == 1800 && _MSC_FULL_VER == 180021114
#  define KE_CXX_HAS_CONSTEXPR
# endif
# if _MSC_VER >= 1900
#  define KE_CXX_HAS_CONSTEXPR
#  define KE_CXX_HAS_NOEXCEPT
#  define KE_CXX_HAS_GENERIC_LAMBDA_CAPTURES
# endif
#else
# error Unrecognized compiler.
#endif

// Done with compiler feature detection.

#if !defined(KE_CXX_HAS_OVERRIDE)
# error "AMTL requires C++11 override"
#endif
#if !defined(KE_CXX_HAS_DELETE)
# error "AMTL requires C++11 method deletion"
#endif
#if !defined(KE_CXX_HAS_EXPLICIT_BOOL)
# error "AMTL requires C++11 explicit bool"
#endif

#if defined(KE_CXX_HAS_NOEXCEPT)
# define KE_NOEXCEPT noexcept
#else
# define KE_NOEXCEPT
#endif

#if defined(KE_CXX_HAS_CONSTEXPR)
# define KE_CONSTEXPR constexpr
#else
# define KE_CONSTEXPR
#endif

#if defined(KE_CXX_HAS_STATIC_ASSERT)
# define KE_STATIC_ASSERT(cond) static_assert(cond, #cond)
#else
# define KE_STATIC_ASSERT(cond) extern int static_assert_f(int a[(cond) ? 1 : -1])
#endif

#if !defined(KE_CXX_HAS_RVAL_REFS) || KE_CXX_HAS_RVAL_REFS < 21
//# error AMTL requires rvalue reference 2.1 support (N2844+)
#endif
#if !defined(KE_CXX_HAS_DOUBLE_GT)
# error AMTL requires support for >> in template names
#endif
#if !defined(KE_CXX_HAS_NULLPTR)
# if defined(__GNUC__) && !defined(__clang__)
#  define nullptr __null
#  define KE_CXX_HAS_NULLPTR
# else
#  error AMTL requires nullptr support
# endif
#endif

#if defined(_MSC_VER)
// This feature has been around for long enough that we shouldn't have to
// warn for it.
# pragma warning(disable: 4351)
#endif

#if defined(_MSC_VER)
# define KE_LINKONCE(x) __declspec(selectany) x
#else
# define KE_LINKONCE(x) x __attribute__((weak))
#endif

#endif // _include_amtl_cxx_support_h_
