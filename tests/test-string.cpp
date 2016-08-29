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

  bool testCreate() const {
    bool ok = true;
    {
      AString str;
      ok &= check(str.length() == 0, "length should = 0");
    }
    {
      AString str("Hello");
      ok &= check(strcmp(str.chars(), "Hello") == 0, "str should be ==\"Hello\"");
      const char* chars = str.chars();
      ok &= check(chars[str.length()] == '\0', "string should be null terminated");
    }
    {
      AString str("test");
      ok &= check(str.length() == 4, "\"test\".length should be == 4");
    }

    return ok;
  }

  bool testOperatorPlus() const {
    bool ok = true;

    AString a("Hello");
    AString b(" ");
    AString c("World");

    ok &= check(
      strcmp((a + b + c).chars(), "Hello World") == 0,
      "a+b+c should be \"Hello World\""
    );
    ok &= check(
      strcmp((a + " World").chars(), "Hello World") == 0,
      "a + \"World\" should be \"Hello World\""
    );
    ok &= check(
      strcmp(("Hello" + b + c).chars(), "Hello World") == 0,
      "\"Hello\" + b + c should be \"Hello World\""
    );
    return ok;
  }

  bool testRValue() const {
    AString a("Hello");
    AString b("World");
    AString c;
    c = a + b;
    return check(
      strcmp(c.chars(), "HelloWorld") == 0,
      "c should be \"HelloWorld\""
    );
  }

  bool testGetSetChar() const {
    bool ok = true;
    AString str("ababaca");
    ok &= check(str[0] == 'a', "str[0] should be == 'a'");

    str[2] = 'b';
    ok &= check(
      strcmp(str.chars(), "abbbaca") == 0,
      "ababaca[2] = 'b' should be abbbaca"
    );
    return ok;
  }

  bool Run() override
  {
    if (!testSprintf())
      return false;
    if (!testCreate())
      return false;
    if (!testOperatorPlus())
      return false;
    if (!testRValue())
      return false;
    if (!testGetSetChar())
      return false;
    return true;
  };
} sTestString;

