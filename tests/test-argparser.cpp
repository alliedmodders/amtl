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

#include <experimental/am-argparser.h>
#include <limits.h>
#include "runner.h"

using namespace ke;
using namespace ke::args;

class TestArgparser : public Test
{
 public:
  TestArgparser()
   : Test("Argparser")
  {
  }

  bool testBasic() {
    Parser parser("help");

    if (!check(parser.parsev(nullptr) == true, "parse no args"))
      return false;
    if (!check(parser.parsev("asdf", nullptr) == false, "error on parse extra args"))
      return false;
    if (!check(parser.parsev("--asdf", nullptr) == false, "error on parse unknown arg"))
      return false;
    return true;
  }

  bool testBoolArg() {
    Parser parser("help");

    BoolOption no_default(parser,
      "b", "bool",
      Nothing(),
      "help");
    BoolOption default_true(parser,
      "t", "default-true",
      Some(true),
      "help");
    BoolOption default_false(parser,
      "f", "default-false",
      Some(false),
      "help");

    if (!check(parser.parsev(nullptr) == true, "parse 1 succeeded"))
      return false;
    if (!check(no_default.hasValue() == false, "no_default has no value"))
      return false;
    if (!check(default_true.hasValue() == true, "default_true has value"))
      return false;
    if (!check(default_true.hasUserValue() == false, "default_true has no user value"))
      return false;
    if (!check(default_true.value() == true, "default_true is true"))
      return false;
    if (!check(default_false.hasValue() == true, "default_false has value"))
      return false;
    if (!check(default_false.hasUserValue() == false, "default_false has no user value"))
      return false;
    if (!check(default_false.value() == false, "default_false is false"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-b", nullptr) == true, "parse 2 succeeded"))
      return false;
    if (!check(no_default.hasValue() == true, "no_default has value"))
      return false;
    if (!check(no_default.value() == true, "no_default is true"))
      return false;
    if (!check(default_true.hasUserValue() == false, "default_true has no user value"))
      return false;
    if (!check(default_false.hasUserValue() == false, "default_false has no user value"))
      return false;

    parser.reset();
    if (!check(parser.parsev("--bool=false", nullptr) == true, "parse 3 succeeded"))
      return false;
    if (!check(no_default.value() == false, "no_default is false"))
      return false;

    parser.reset();
    if (!check(parser.parsev("--bool", "false", nullptr) == true, "parse 4 succeeded"))
      return false;
    if (!check(no_default.value() == false, "no_default is false"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-b", "true", nullptr) == true, "parse 5 succeeded"))
      return false;
    if (!check(no_default.value() == true, "no_default is false"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-f", nullptr) == true, "parse 6 succeeded"))
      return false;
    if (!check(default_false.value() == true, "default_false is true"))
      return false;

    parser.reset();
    if (!check(parser.parsev("--default-true=false", nullptr) == true, "parse 7 succeeded"))
      return false;
    if (!check(default_true.value() == false, "default_true is false"))
      return false;

    return true;
  }

  bool testToggleArg() {
    Parser parser("help");

    ToggleOption x(parser,
      "x", nullptr,
      Nothing(),
      "help");
    ToggleOption y(parser,
      "y", nullptr,
      Some(true),
      "help");
    ToggleOption z(parser,
      "z", nullptr,
      Some(false),
      "help");

    parser.reset();
    if (!check(parser.parsev(nullptr) == true, "toggle parse 1 succeeded"))
      return false;
    if (!check(x.value() == false, "x is false"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-x", nullptr) == true, "toggle parse 2 succeeded"))
      return false;
    if (!check(x.value() == true, "x is true"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-y", nullptr) == true, "toggle parse 3 succeeded"))
      return false;
    if (!check(y.value() == false, "y is false"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-z", nullptr) == true, "toggle parse 4 succeeded"))
      return false;
    if (!check(z.value() == true, "z is true"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-z=false", nullptr) == false, "toggle parse 5 failed"))
      return false;

    return true;
  }

  bool testStringArg() {
    Parser parser("help");

    StringOption s(parser,
      "s", "string",
      Nothing(),
      "help");
    StringOption t(parser,
      "t", "ttt",
      Some(AString("whatever")),
      "help");
    StringOption mode(parser, "mode", "help");

    parser.reset();
    if (!check(parser.parsev(nullptr) == false, "string parse 1 failed"))
      return false;

    parser.reset();
    if (!check(parser.parsev("crab", nullptr) == true, "string parse 2 succeeded"))
      return false;
    if (!check(s.hasValue() == false, "string is false"))
      return false;
    if (!check(t.value().compare("whatever") == 0, "ttt is 'whatever'"))
      return false;
    if (!check(mode.value().compare("crab") == 0, "mode is 'crab'"))
      return false;

    parser.reset();
    if (!check(parser.parsev("-s", "yam", "egg", nullptr) == true, "string parse 3 succeeded"))
      return false;
    if (!check(s.value().compare("yam") == 0, "string is 'yam'"))
      return false;
    if (!check(mode.value().compare("egg") == 0, "mode is 'crab'"))
      return false;

    return true;
  }

  bool testIntArg() {
    Parser parser("help");

    IntOption val1(parser,
      nullptr, "val",
      Nothing(),
      "help");

    parser.reset();
    if (!check(parser.parsev("--val", "308", nullptr) == true, "int parse 1 succeeded"))
      return false;
    if (!check(val1.value() == 308, "value is 308"))
      return false;

    parser.reset();
    if (!check(parser.parsev("--val", "30x", nullptr) == false, "int parse 2 failed"))
      return false;

    return true;
  }

  bool testRepeatArg() {
    Parser parser("help");

    RepeatOption<StringValue> inc(parser,
      "-i", "--include-path",
      "Include path.");

    if (!check(parser.parsev("-i", "blah", "-i", "crab", "--include-path=yam", nullptr) == true,
               "repeat parse 1 succeeded"))
    {
      parser.usage(stderr, 0, nullptr);
      return false;
    }

    Vector<AString> values = Move(inc.values());
    if (!check(values.length() == 3, "values should be 3"))
      return false;
    if (!check(values[0].compare("blah") == 0, "value 0 should be blah"))
      return false;
    if (!check(values[1].compare("crab") == 0, "value 1 should be crab"))
      return false;
    if (!check(values[2].compare("yam") == 0, "value 2 should be yam"))
      return false;
    return true;
  }

  bool Run() override
  {
    if (!testBasic())
      return false;
    if (!testBoolArg())
      return false;
    if (!testToggleArg())
      return false;
    if (!testStringArg())
      return false;
    if (!testIntArg())
      return false;
    if (!testRepeatArg())
      return false;
    return true;
  };
} sTestArgparser;

