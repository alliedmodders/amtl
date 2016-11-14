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

#ifndef _include_amtl_memory_h_
#define _include_amtl_memory_h_

#include <amtl/am-platform.h> // Windows CRT memory leak detector requires a specific header include order.
#include <stdlib.h>
#if defined(KE_WINDOWS)
# include <crtdbg.h>

# if defined(_DEBUG)
#  define KE_MAP_CRTALLOC
# endif // _DEBUG
#endif // KE_WINDOWS

namespace ke {

#if defined(KE_MAP_CRTALLOC)
  static inline void* am_malloc_dbg(size_t size, int blockuse, const char* filename, int linenumber) {
    return _malloc_dbg(size, blockuse, filename, linenumber);
  }

  static inline void* am_calloc_dbg(size_t count, size_t size, int blockuse, const char* filename, int linenumber) {
    return _calloc_dbg(count, size, blockuse, filename, linenumber);
  }

  static inline void* am_realloc_dbg(void* ptr, size_t size, int blockuse, const char* filename, int linenumber) {
    return _realloc_dbg(ptr, size, blockuse, filename, linenumber);
  }

  static inline void am_free_dbg(void* ptr, int blockuse) {
    _free_dbg(ptr, blockuse);
  }

# if defined(_malloc_dbg)
#  undef _malloc_dbg
# endif // _malloc_dbg
# define _malloc_dbg ke::am_malloc_dbg
# if defined(_calloc_dbg)
#  undef _calloc_dbg
# endif // _calloc_dbg
# define _calloc_dbg ke::am_calloc_dbg
# if defined(_realloc_dbg)
#  undef _realloc_dbg
# endif // _realloc_dbg
# define _realloc_dbg ke::am_realloc_dbg
# if defined(_free_dbg)
#  undef _free_dbg
# endif // _free_dbg
# define _free_dbg ke::am_free_dbg
#endif // KE_MAP_CRTALLOC

  static inline void* am_malloc(size_t size) {
    return malloc(size);
  }

  static inline void* am_calloc(size_t count, size_t size) {
    return calloc(count, size);
  }

  static inline void* am_realloc(void* ptr, size_t size) {
    return realloc(ptr, size);
  }

  static inline void am_free(void* ptr) {
    free(ptr);
  }

# if defined(malloc)
#  undef malloc
# endif // malloc
# define malloc ke::am_malloc
# if defined(calloc)
#  undef calloc
# endif // calloc
# define calloc ke::am_calloc
# if defined(realloc)
#  undef realloc
# endif // realloc
# define realloc ke::am_realloc
# if defined(free)
#  undef free
# endif // free
# define free ke::am_free

}

#if defined(KE_MAP_CRTALLOC)
# undef KE_MAP_CRTALLOC
#endif // KE_MAP_CRTALLOC

#endif // _include_amtl_memory_h_
