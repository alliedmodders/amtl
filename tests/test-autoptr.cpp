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

#include <am-autoptr.h>
#include "runner.h"

using namespace ke;

static size_t sBlahCtors = 0;
static size_t sBlahDtors = 0;
struct Blah {
  Blah() {
    sBlahCtors++;
  }
  ~Blah() {
    sBlahDtors++;
  }
};

static size_t sDeleterCalls = 0;
static size_t sDeleterMoves = 0;
template <typename T>
class CountedDeleter
{
public:
  CountedDeleter &operator=(CountedDeleter &&) {
    ++sDeleterMoves;
    return *this;
  }

public:
  void operator()(typename remove_extent<T>::type *ptr) {
    if (ptr)
      ++sDeleterCalls;

    DefaultDeleter<T>{}(ptr);
  }
};

class TestAutoPtr : public Test
{
 public:
  TestAutoPtr()
   : Test("AutoPtr")
  {
  }

  bool Run() override
  {
    if (!testSingle())
      return false;
    else if (!testDeleter())
      return false;
    return true;
  };

 private:
  bool testSingle() {
    AutoPtr<int> intp(new int(5));
    if (!check(*intp.get() == 5, "pointer should contain 5"))
      return false;

    intp = MakeUnique<int>(7);
    if (!check(*intp.get() == 7, "pointer should contain 7"))
      return false;

    intp = new int(2);
    if (!check(*intp.get() == 2, "pointer should contain 2"))
      return false;

    {
      AutoPtr<Blah> blah(new Blah());
      if (!check(sBlahCtors == 1, "called Blah::Blah"))
        return false;
    }
    if (!check(sBlahDtors == 1, "called Blah::~Blah"))
      return false;

    sBlahCtors = 0;
    sBlahDtors = 0;
    {
      AutoPtr<Blah[]> blah(new Blah[20]);
      if (!check(sBlahCtors == 20, "called Blah::Blah 20 times"))
        return false;

      blah = MakeUnique<Blah[]>(15);
    }
    if (!check(sBlahDtors == 35, "called Blah::~Blah 35 times"))
      return false;

    return true;
  }

  bool testDeleter() {
    {
      ke::AutoPtr<int, CountedDeleter<int>> ptr(MakeUnique<int>(6));
      {
        ke::AutoPtr<int, CountedDeleter<int>> ptr2(MakeUnique<int>(7));
        ptr = Move(ptr2);
        if (!check(sDeleterCalls == 1, "deletion count should be 1"))
          return false;
        else if (!check(sDeleterMoves == 1, "deletor moved once"))
          return false;
      }

      if (!check(sDeleterCalls == 1, "deletion count should still be 1"))
        return false;
    }

    if (!check(sDeleterCalls == 2, "deletion count should be 2"))
      return false;

    sDeleterCalls = 0;
    sDeleterMoves = 0;

    {
      ke::AutoPtr<int[], CountedDeleter<int[]>> ptr(new int[2]);
      {
        ke::AutoPtr<int[], CountedDeleter<int[]>> ptr2(new int[3]);
        ptr = Move(ptr2);
        if (!check(sDeleterCalls == 1, "array deletion count should be 1"))
          return false;
        else if (!check(sDeleterMoves == 1, "array deletor moved once"))
          return false;
      }

      if (!check(sDeleterCalls == 1, "array deletion count should still be 1"))
        return false;
    }

    if (!check(sDeleterCalls == 2, "array deletion count should be 2"))
      return false;

    return true;
  }

} sTestAutoPtr;
