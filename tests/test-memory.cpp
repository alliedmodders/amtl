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

#include <am-memory.h>
#include "runner.h"

using namespace ke;

class TestMemory : public Test
{
public:
  TestMemory()
    : Test("Memory")
  {
  }

  bool test_malloc()
  {
    MemoryTracker::void_pointer memory;
    
    if (!check((memory = malloc(sizeof(int))) != nullptr, "Allocate %lu bytes of memory", sizeof(int)))
      return false;

    free(memory);

    return true;
  }

  bool test_calloc()
  {
    MemoryTracker::void_pointer memory;
    MemoryTracker::size_type size = 10;

    if (!check((memory = calloc(size, sizeof(int))) != nullptr, "Allocate %lu bytes of memory %lu times", sizeof(int), size))
      return false;

    free(memory);

    return true;
  }

  bool test_realloc()
  {
    MemoryTracker::void_pointer memory, old_memory;

    if (!check_silent((memory = malloc(sizeof(int))) != nullptr, "Allocate %lu bytes of memory", sizeof(int)))
      return false;

    old_memory = memory;

    if (!check((memory = realloc(memory, sizeof(int) * 2)) != nullptr, "Reallocate %p to %lu bytes of memory", old_memory, sizeof(int) * 2))
      return false;

    free(memory);

    return true;
  }
  
  bool test_memsize()
  {
    MemoryTracker::void_pointer memory;

    if (!check_silent((memory = malloc(sizeof(int))) != nullptr, "Allocate %lu bytes of memory", sizeof(int)))
      return false;
    else if (!check(MemoryTracker::GetAllocater().memory_size(memory) == sizeof(int), "Size of %p should be %lu", memory, sizeof(int)))
      return false;
    
    free(memory);
    
    return true;
  }

  bool test_recalloc()
  {
    MemoryTracker::void_pointer memory, old_memory;
    MemoryTracker::size_type size = 10;

    if (!check_silent((memory = calloc(size, sizeof(int))) != nullptr, "Allocate %lu bytes of memory %lu times", sizeof(int), size))
      return false;

    old_memory = memory;

    if (!check((memory = MemoryTracker::GetAllocater().recallocate(memory, 2, sizeof(int))) != nullptr, "Reallocate %p to %lu bytes of memory %lu times", old_memory, sizeof(int), 2))
      return false;

    free(memory);

    return true;
  }

  bool Run() override
  {
    if (!test_malloc())
      return false;
    else if (!test_calloc())
      return false;
    else if (!test_realloc())
      return false;
    else if (!test_memsize())
      return false;
    else if (!test_recalloc())
      return false;
    return true;
  }
} sTestMemory;
