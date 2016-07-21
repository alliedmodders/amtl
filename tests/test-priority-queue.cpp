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

#include <amtl/am-priority-queue.h>
#include <assert.h>
#include "runner.h"

using namespace ke;

class TestPriorityQueue : public Test
{
 public:
  TestPriorityQueue()
   : Test("PriorityQueue")
  {
  }

  bool testBasic()
  {
    PriorityQueue<int> pq;
    pq.add(16);
    pq.add(16);
    pq.add(16);
    pq.add(9);
    pq.add(77);
    pq.add(3);

    if (!check(pq.popCopy() == 3, "should pop 3"))
      return false;
    if (!check(pq.popCopy() == 9, "should pop 9"))
      return false;
    if (!check(pq.popCopy() == 16, "should pop 16"))
      return false;
    if (!check(pq.popCopy() == 16, "should pop 16"))
      return false;
    if (!check(pq.popCopy() == 16, "should pop 16"))
      return false;
    if (!check(pq.popCopy() == 77, "should pop 77"))
      return false;
    if (!check(pq.empty(), "should be empty"))
      return false;

    return true;
  }

  bool Run() override
  {
    if (!testBasic())
      return false;
    return true;
  }
} sTestPriorityQueue;

