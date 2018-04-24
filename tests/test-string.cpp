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

#include <am-string.h>
#include <limits.h>
#include "runner.h"

using namespace ke;

class TestString : public Test
{
 public:
  TestString()
   : Test("String")
  {
  }

  bool testSprintf() const {
    int64_t x = 1234;
    char buffer[256];
    SafeSprintf(buffer, sizeof(buffer), "%" KE_FMT_I64, x);
    if (!check(strcmp(buffer, "1234") == 0, "buffer should == \"1234\""))
      return false;
    return true;
  }

  bool testAllocating() const {
    int a = INT_MAX;
    const char* value = "Hello this is a test.";
    const char* expect = "A: 2147483647 B: Hello this is a test.";
    UniquePtr<char[]> ptr = Sprintf("A: %d B: %s", a, value);
    if (!check(strcmp(ptr.get(), expect) == 0, expect))
      return false;

    UniquePtr<AString> str = AString::Sprintf("A: %d B: %s", a, value);
    if (!check(str->compare(expect) == 0, expect))
      return false;
    return true;
  }

  bool Run() override
  {
    if (!testSprintf())
      return false;
    if (!testAllocating())
      return false;
    return true;
  };
} sTestString;

