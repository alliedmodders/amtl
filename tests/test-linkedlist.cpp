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

#include <am-linkedlist.h>
#include <assert.h>
#include "runner.h"

using namespace ke;

static size_t sCtors = 0;
static size_t sCopyCtors = 0;
static size_t sMovingCtors = 0;
static size_t sMovedDtors = 0;
static size_t sDtors = 0;

namespace {

class BasicThing
{
 public:
  BasicThing()
  {
    sCtors++;
  }
  BasicThing(const BasicThing &other)
  {
    sCopyCtors++;
  }
  ~BasicThing()
  {
    sDtors++;
  }
};

class MovingThing
{
 public:
  MovingThing()
   : moved_(false)
  {
    sCtors++;
  }
  MovingThing(MovingThing &&other)
  {
    assert(!other.moved_);
    sMovingCtors++;
    moved_ = false;
    other.moved_ = true;
  }
  ~MovingThing()
  {
    sDtors++;
    if (moved_)
      sMovedDtors++;
  }

 private:
  MovingThing(const MovingThing &other) KE_DELETE;
  bool moved_;
};

} // anonymous namespace

class TestLinkedList : public Test
{
 public:
  TestLinkedList()
   : Test("LinkedList")
  {
  }

  bool testInts()
  {
    LinkedList<int> list;

    if (!check(list.empty(), "list should be empty"))
      return false;
    if (!check(list.begin() == list.end(), "list iterator should be empty"))
      return false;

    list.append(5);
    list.append(7);
    list.append(9);
    if (!check(!list.empty(), "list should not be empty"))
      return false;
    if (!check(list.length() == 3, "list size should be 3"))
      return false;

    LinkedList<int>::iterator iter = list.begin();
    if (!check(iter != list.end(), "list iterator should not have ended"))
      return false;
    if (!check(*iter == 5, "item 1 should be 5"))
      return false;
    iter++;
    if (!check(iter != list.end(), "list iterator should not have ended"))
      return false;
    if (!check(*iter == 7, "item 2 should be 7"))
      return false;
    iter++;
    if (!check(iter != list.end(), "list iterator should not have ended"))
      return false;
    if (!check(*iter == 9, "item 3 should be 9"))
      return false;
    iter++;
    if (!check(iter == list.end(), "list iterator should have ended"))
      return false;

    iter = list.begin();
    iter++;
    iter++;
    iter = list.erase(iter);
    if (!check(iter == list.end(), "erasure should have returned ended iterator"))
      return false;
    list.append(11);

    iter = list.begin();
    iter = list.erase(iter);
    if (!check(iter != list.end(), "erasure should not have ended iterator"))
      return false;
    if (!check(*iter == 7, "erasure should have returned item 2"))
      return false;
    iter++;
    if (!check(*iter == 11, "second item should be 11"))
      return false;
    if (!check(list.length() == 2, "list should have two items"))
      return false;

    list.prepend(1);
    iter = list.begin();
    if (!check(*iter == 1, "new item should have been prepended"))
      return false;
    iter++;
    if (!check(*iter == 7, "second item should be 7"))
      return false;
    if (!check(list.length() == 3, "list should have three items"))
      return false;

    list.clear();
    if (!check(list.empty(), "cleared list should be empty"))
      return false;
    if (!check(list.begin() == list.end(), "cleared list iterator should be empty"))
      return false;

    list.append(1);
    list.append(2);
    list.append(3);
    list.append(4);
    list.append(5);
    list.append(5);
    iter = list.find(3);
    if (!check(iter != list.end(), "should have found a 3"))
      return false;
    if (!check(*iter == 3, "should have found 3"))
      return false;
    list.remove(3);
    iter = list.find(3);
    if (!check(iter == list.end(), "should not have found a 3"))
      return false;
    iter = list.find(5);
    if (!check(iter != list.end(), "should have found a 5"))
      return false;
    if (!check(*iter == 5, "should have found 5"))
      return false;
    list.remove(5);
    iter = list.find(5);
    if (!check(iter != list.end(), "should have found a second 5"))
      return false;
    if (!check(*iter == 5, "should have found a second 5"))
      return false;
    list.remove(5);
    list.remove(5);
    iter = list.find(5);
    if (!check(iter == list.end(), "should not have found a third 5"))
      return false;

    return true;
  }

  bool testDestructors()
  {
    {
      LinkedList<BasicThing> list;
      list.append(BasicThing());
      list.append(BasicThing());
      list.append(BasicThing());
    }
    if (!check(sCtors == 3, "should get 3 normal constructors"))
      return false;
    if (!check(sCopyCtors == 3, "should get 3 copy constructors"))
      return false;
    if (!check(sDtors == 6, "should get 6 destructors"))
      return false;

    sCtors = 0;
    sCopyCtors = 0;
    sDtors = 0;
    return true;
  }

  bool testMoving()
  {
    {
      LinkedList<MovingThing> list;
      MovingThing a, b, c;
      list.append(ke::Move(a));
      list.append(ke::Move(b));
      list.append(ke::Move(c));
    }

    if (!check(sCtors == 3, "should get 3 normal constructors"))
      return false;
    if (!check(sMovingCtors == 3, "sould get 3 move constructors"))
      return false;
    if (!check(sMovedDtors == 3, "sould get 3 moved destructors"))
      return false;
    if (!check(sDtors == 6, "should get 6 destructors"))
      return false;

    sCtors = 0;
    sMovingCtors = 0;
    sMovedDtors = 0;
    sDtors = 0;
    return true;
  }

  bool testFallibleMalloc()
  {
    LinkedList<int, FallibleMalloc> list;
    list.append(5);
    list.append(6);
    list.setOutOfMemory(true);
    if (!check(!list.append(7), "list handled out-of-memory"))
      return false;
    list.setOutOfMemory(false);
    if (!check(list.append(8), "list recovered out-of-memory"))
      return false;
    if (!check(list.ooms() == 1, "list received 1 oom"))
      return false;
    return true;
  }

  bool Run() KE_OVERRIDE
  {
    if (!testInts())
      return false;
    if (!testDestructors())
      return false;
    if (!testMoving())
      return false;
    if (!testFallibleMalloc())
      return false;
    return true;
  }
} sTestLinkedList;

