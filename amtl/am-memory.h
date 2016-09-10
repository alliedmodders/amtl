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

#include <am-platform.h>
#include <am-thread-utils.h>
#ifdef KE_WINDOWS
# undef _CRTDBG_MAP_ALLOC // am-memory.h tracks memory like the debug versions of malloc/free
# include <crtdbg.h>
#endif // KE_WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <am-bits.h>

namespace ke {

  class MemoryTracker
  {
  public:
    typedef size_t size_type;
    typedef void *void_pointer;
  private:
    MemoryTracker() :
      memory_(nullptr),
      memorySize_(nullptr),
      maxsize_(0),
      memlocker_()
    {
    }
  public:
    ~MemoryTracker()
    {
      ke::AutoLock memLock(&memlocker_);

      for (size_type i = 0; i < maxsize_; ++i)
      {
        if (memory_[i]) // Leaked memory
        {
          fprintf(stderr, "Freeing %lu bytes of leaked memory at %p", memorySize_[i], memory_[i]);
          free(memory_[i]);
        }
      }

      free(memory_);
      free(memorySize_);
    }
  private:
    MemoryTracker(const MemoryTracker &other) = delete;
    MemoryTracker(MemoryTracker &&other) = delete;
    MemoryTracker &operator=(const MemoryTracker &other) = delete;
    MemoryTracker &operator=(MemoryTracker &&other) = delete;
  public:
    static MemoryTracker &GetAllocater()
    {
      static MemoryTracker memoryAllocator;

      return memoryAllocator;
    }
  public:
    size_type memory_size(void_pointer memory)
    {
      ke::AutoLock memLock(&memlocker_);

      size_type index;
      if (!find(memory, index))
        return 0;

      return memorySize_[index];
    }

    void_pointer allocate(size_type bytes)
    {
      ke::AutoLock memLock(&memlocker_);

      size_type index;
      if (!grow(index))
        return nullptr;

      void_pointer memory = malloc(bytes);
      if (!memory)
      {
        reportOutOfMemory();
        return nullptr;
      }

      memory_[index] = memory;
      memorySize_[index] = bytes;

      return memory;
    }

    void deallocate(void_pointer memory)
    {
      ke::AutoLock memLock(&memlocker_);

      size_type index;
      if (!find(memory, index)) // Allocate methods has matching de-allocate method, enforce that.
        return;

      free(memory);
      memory_[index] = nullptr;
      memorySize_[index] = 0;
    }

    void_pointer callocate(size_type count, size_type bytes)
    {
      return allocate(count * bytes);
    }

    void_pointer reallocate(void_pointer memory, size_type bytes)
    {
      ke::AutoLock memLock(&memlocker_);

      size_type index;
      if (!find(memory, index)) // Only re-allocate memory we allocated.
        return nullptr;

      void_pointer newmemory = malloc(bytes);
      if (!newmemory)
      {
        reportOutOfMemory();
        return nullptr;
      }

      memcpy(newmemory, memory, memorySize_[index] <= bytes ? memorySize_[index] : bytes);
      free(memory);
      memory_[index] = newmemory;
      memorySize_[index] = bytes;

      return newmemory;
    }

    void_pointer recallocate(void_pointer memory, size_type count, size_type bytes)
    {
      return reallocate(memory, count * bytes);
    }
  public:
    void reportAllocationOverflow()
    {
      fprintf(stderr, "Unable to allocate memory, max value of size_t reached");
      abort();
    }

    void reportOutOfMemory()
    {
      fprintf(stderr, "Unable to allocate memory, system ran out of memory");
      abort();
    }
  private:
    bool find(void_pointer memory, size_type &index) const
    {
      index = 0;

      for (size_type i = 0; i < maxsize_; ++i)
      {
        if (memory_[i] == memory)
        {
          index = i;
          return true;
        }
      }

      return false;
    }

    bool grow(size_type &index)
    {
      if (find(nullptr, index))
        return true;
      else if (!IsUintPtrAddSafe(maxsize_, 1))
      {
        reportAllocationOverflow();
        return false;
      }

      void_pointer *newmemory = (void_pointer *)malloc(sizeof(void_pointer) * (maxsize_ + 1));
      if (!newmemory)
      {
        reportOutOfMemory();
        return false;
      }

      size_type *newsize = (size_type *)malloc(sizeof(size_type) * (maxsize_ + 1));
      if (!newmemory)
      {
        free(newmemory);
        reportOutOfMemory();
        return false;
      }

      if (maxsize_)
      {
        memcpy(newmemory, memory_, maxsize_);
        memcpy(newsize, memorySize_, maxsize_);
      }

      free(memory_);
      free(memorySize_);

      memory_ = newmemory;
      memorySize_ = newsize;
      index = maxsize_++;

      return true;
    }
  private:
    void_pointer *memory_;
    size_type *memorySize_;
    size_type maxsize_;
    Mutex memlocker_;
  };

  static inline MemoryTracker::void_pointer
  am_malloc(MemoryTracker::size_type bytes)
  {
    return MemoryTracker::GetAllocater().allocate(bytes);
  }

  static inline MemoryTracker::void_pointer
  am_calloc(MemoryTracker::size_type count, MemoryTracker::size_type bytes)
  {
    return MemoryTracker::GetAllocater().callocate(count, bytes);
  }

  static inline MemoryTracker::void_pointer
  am_realloc(MemoryTracker::void_pointer memory, MemoryTracker::size_type bytes)
  {
    return MemoryTracker::GetAllocater().reallocate(memory, bytes);
  }

  static inline void
  am_free(MemoryTracker::void_pointer memory)
  {
    MemoryTracker::GetAllocater().deallocate(memory);
  }

}

#undef malloc
#define malloc(s) ke::am_malloc(s)
#undef calloc
#define calloc(c, s) ke::am_calloc(c, s)
#undef realloc
#define realloc(p, s) ke::am_realloc(p, s)
#undef free
#define free(p) ke::am_free(p)

#endif // _include_amtl_memory_h_
