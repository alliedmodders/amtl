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

#include <am-vector.h>
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
  MovingThing(Moveable<MovingThing> other)
  {
    assert(!other->moved_);
    sMovingCtors++;
    moved_ = false;
    other->moved_ = true;
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

class TestVector : public Test
{
 public:
  TestVector()
   : Test("Vector")
  {
  }

  bool testInts()
  {
    Vector<int> vector;

    if (!check(vector.empty(), "vector should be empty"))
      return false;
    if (!check(vector.length() == 0, "vector length should be 0"))
      return false;

    vector.append(1);
    vector.append(2);
    vector.append(3);
    vector.append(4);
    vector.append(5);
    for (int i = 0; i < 5; i++) {
      if (!check(vector[i] == i + 1, "vector[%d] is %d", i, i))
        return false;
    }
    if (!check(vector.length() == 5, "vector length is 5"))
      return false;
    if (!check(!vector.empty(), "vector is not empty"))
      return false;

    if (!check(vector.popCopy() == 5, "popped 5"))
      return false;
    if (!check(vector.popCopy() == 4, "popped 4"))
      return false;
    if (!check(vector.length() == 3, "vector length is 3"))
      return false;

    vector.insert(0, 88);
    vector.insert(0, 99);
    vector.insert(4, 111);
    if (!check(vector[0] == 99, "[0] should be 99"))
      return false;
    if (!check(vector[1] == 88, "[1] should be 88"))
      return false;
    if (!check(vector[2] == 1, "[2] should be 1"))
      return false;
    if (!check(vector[3] == 2, "[3] should be 2"))
      return false;
    if (!check(vector[4] == 111, "[4] should be 111"))
      return false;
    if (!check(vector[5] == 3, "[5] should be 3"))
      return false;
    if (!check(vector.length() == 6, "length should be 6"))
      return false;

    vector.remove(5);
    if (!check(vector[4] == 111, "[4] should still be 111"))
      return false;
    vector.remove(0);
    if (!check(vector[0] == 88, "[0] should be 88"))
      return false;
    if (!check(vector[3] == 111, "[3] should be 111"))
      return false;
    if (!check(vector.length() == 4, "length should be 4"))
      return false;

    while (!vector.empty())
      vector.pop();

    if (!check(vector.empty(), "vector should be empty"))
      return false;
    if (!check(vector.ensure(128), "vector size ensured"))
      return false;
    if (!check(vector.empty(), "vector should be empty"))
      return false;

    for (int i = 0; i < 128; i++)
      vector.append(i);
    if (!check(vector.length() == 128, "vector length should be 128"))
      return false;

    vector.clear();
    if (!check(vector.empty(), "vector should be empty"))
      return false;

    return true;
  }

  bool testDestructors()
  {
    {
      Vector<BasicThing> vector;
      vector.append(BasicThing());
      vector.append(BasicThing());
      vector.append(BasicThing());
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
      Vector<MovingThing> vector;
      MovingThing a, b, c;
      vector.append(Moveable<MovingThing>(a));
      vector.append(Moveable<MovingThing>(b));
      vector.append(Moveable<MovingThing>(c));
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

    Vector<int> v1;
    v1.append(10);
    Vector<int> v2(Move(v1));
    if (!check(v2.length() == 1, "should have moved vector"))
      return false;
    if (!check(v1.length() == 0, "should have gutted vector"))
      return false;

    return true;
  }

  bool testFallibleMalloc()
  {
    Vector<int, FallibleMalloc> vector;
    vector.setOutOfMemory(true);
    if (!check(!vector.append(7), "vector handled out-of-memory"))
      return false;
    vector.setOutOfMemory(false);
    if (!check(vector.append(8), "vector recovered out-of-memory"))
      return false;
    if (!check(vector.ooms() == 1, "vector received 1 oom"))
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
} sTestVector;

