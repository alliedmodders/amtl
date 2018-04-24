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

  bool testSplit() const {
    Vector<AString> out = Split("     ", " ");
    if (!check(out.length() == 6, "length should be 6"))
      return false;
    for (size_t i = 0; i < out.length(); i++) {
      if (!check(out[i].length() == 0, "string should be empty"))
        return false;
    }

    out = Split("egg", " ");
    if (!check(out.length() == 1, "length should be 1"))
      return false;
    if (!check(out[0].compare("egg") == 0, "item should be egg"))
      return false;

    out = Split("", "egg");
    if (!check(out.length() == 0, "length should be 0"))
      return false;

    out = Split("xaba", "a");
    if (!check(out.length() == 3, "length should be 3"))
      return false;
    if (!check(out[0].compare("x") == 0, "first item should be 'x'"))
      return false;
    if (!check(out[1].compare("b") == 0, "second item should be 'b'"))
      return false;
    if (!check(out[2].compare("") == 0, "third item should be empty"))
      return false;

    out = Split("egg ham", " ");
    if (!check(out.length() == 2, "length should be 2"))
      return false;
    if (!check(out[0].compare("egg") == 0, "first item should be 'egg'"))
      return false;
    if (!check(out[1].compare("ham") == 0, "second item should be 'ham'"))
      return false;

    return true;
  }

  bool testJoin() const {
    Vector<AString> in;

    AString result = Join(in, "x");
    if (!check(result.compare("") == 0, "string should be empty"))
      return false;

    in.append("abc");
    result = Join(in, "x");
    if (!check(result.compare("abc") == 0, "string should be 'abc'"))
      return false;

    in.append("xyz");
    result = Join(in, "T");
    if (!check(result.compare("abcTxyz") == 0, "string should be 'abcTxyz'"))
      return false;

    in.append("def");
    result = Join(in, "");
    if (!check(result.compare("abcxyzdef") == 0, "string should be 'abcxyzdef'"))
      return false;

    return true;
  }

  bool testCase() {
    AString str("samPle1.com");
    str = str.uppercase();
    if (!check(str.compare("SAMPLE1.COM") == 0, "string should be uppercase"))
      return false;

    str = str.lowercase();
    if (!check(str.compare("sample1.com") == 0, "string should be lowercase"))
      return false;

    str = AString();
    str = str.lowercase();
    if (!check(str.compare("") == 0, "empty string"))
      return false;
    return true;
  }

  bool Run() override
  {
    if (!testSprintf())
      return false;
    if (!testAllocating())
      return false;
    if (!testSplit())
      return false;
    if (!testJoin())
      return false;
    if (!testCase())
      return false;
    return true;
  };
} sTestString;

