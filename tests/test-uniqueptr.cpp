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

#include <am-uniqueptr.h>
#include "runner.h"

using namespace ke;

static size_t sBrahCtors = 0;
static size_t sBrahDtors = 0;
struct Brah {
  Brah() {
    sBrahCtors++;
  }
  ~Brah() {
    sBrahDtors++;
  }
};

class TestUniquePtr : public Test
{
 public:
  TestUniquePtr()
   : Test("UniquePtr")
  {
  }

  bool Run() override
  {
    if (!testSingle())
      return false;
    return true;
  };

 private:
  bool testSingle() {
    UniquePtr<int> five = MakeUnique<int>(5);
    if (!check(*five.get() == 5, "pointer should contain 5"))
      return false;

    five = nullptr;
    if (!check(!five, "pointer should be null"))
      return false;

    {
      UniquePtr<Brah> blah = MakeUnique<Brah>();
      if (!check(sBrahCtors == 1, "called Brah::Brah"))
        return false;
    }
    if (!check(sBrahDtors == 1, "called Brah::~Brah"))
      return false;

    sBrahCtors = 0;
    sBrahDtors = 0;
    {
      UniquePtr<Brah[]> blah = MakeUnique<Brah[]>(20);
      if (!check(sBrahCtors == 20, "called Brah::Brah 20 times"))
        return false;
    }
    if (!check(sBrahDtors == 20, "called Brah::~Brah 20 times"))
      return false;

    return true;
  }

} sTestUniquePtr;
