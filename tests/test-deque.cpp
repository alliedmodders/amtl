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
#include <am-deque.h>
#include <assert.h>
#include "runner.h"

using namespace ke;

class TestDeque : public Test
{
 public:
  TestDeque()
   : Test("Deque")
  {
  }

  bool test_basic() {
    Deque<int> dq;

    for (int i = 0; i < 4; i++) {
      if (!check(dq.append(i), "append %d", i))
        return false;
      if (!check(dq.length() == size_t(i + 1), "length should be %d", i + 1))
        return false;
    }
    for (int i = 0; i < 4; i++) {
      if (!check(dq.prepend(i + 4), "prepend %d", i))
        return false;
      if (!check(dq.length() == size_t(i + 5), "length should be %d", i + 5))
        return false;
    }

    if (!check(dq.back() == 3, "back() should == 3"))
      return false;
    if (!check(dq.front() == 7, "front() should == 7"))
      return false;
    dq.popBack();
    dq.popFront();
    if (!check(dq.popFrontCopy() == 6, "popFrontCopy() should == 5"))
      return false;
    if (!check(dq.popBackCopy() == 2, "popFrontCopy() should == 1"))
      return false;

    if (!check(dq.length() == 4, "length should be 4"))
      return false;

    while (!dq.empty())
      dq.popBack();
    if (!check(dq.length() == 0, "length should be 0"))
      return false;

    return true;
  }

  bool test_prepend_empty() {
    Deque<int> dq;

    for (int i = 0; i < 8; i++) {
      if (i % 2 == 0) {
        if (!check(dq.prepend(i), "prepend %d", i))
          return false;
      } else {
        if (!check(dq.append(i), "append %d", i))
          return false;
      }
      if (!check(dq.length() == size_t(i + 1), "length should be %d", i + 1))
        return false;
    }

    while (!dq.empty())
      dq.popFront();
    if (!check(dq.length() == 0, "length should be 0"))
      return false;

    return true;
  }

  bool test_resize() {
    Deque<int> dq;
    for (int i = 0; i < 387; i++) {
      if (!check_silent(dq.prepend(i), "prepend"))
        return false;
    }
    for (int i = 0; i < 293; i++) {
      if (!check_silent(dq.append(i), "append"))
        return false;
    }

    if (!check(dq.length() == 293 + 387, "length should be %d", 293 + 387))
      return false;

    for (int i = 292; i >= 0; i--) {
      if (!check_silent(dq.popBackCopy() == i, "popBack %d", i))
        return false;
    }
    for (int i = 386; i >= 0; i--) {
      if (!check_silent(dq.popFrontCopy() == i, "popFront %d", i))
        return false;
    }

    return true;
  }

  bool Run() KE_OVERRIDE {
    if (!test_basic())
      return false;
    if (!test_prepend_empty())
      return false;
    if (!test_resize())
      return false;
    return true;
  }
} sTestDeque;

