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

#include <am-function.h>
#include <am-vector.h>
#include "runner.h"

using namespace ke;

static int test_old_fn(int x) {
  return 99 + x;
}

class MoveObj
{
public:
  static unsigned sNumMoves;

  MoveObj() {
    sNumMoves = 0;
  }
  MoveObj(MoveObj&& other) {
    sNumMoves++;
  }

  unsigned count() const {
    return sNumMoves;
  }

private:
  MoveObj(const MoveObj&) = delete;
  void operator =(const MoveObj&) = delete;
};
unsigned MoveObj::sNumMoves = 0;

class CallableObj
{
public:
  int member_function(int x) const {
    return x * 6;
  }
  int operator()(int x) const {
    return x + 34;
  }
};

class TestCallable : public Test
{
public:
  TestCallable()
    : Test("Callable")
  {
  }

  bool testCallable(const Callable<int(int)>& inputCallable, int param, int result) {
    if (!check_silent((bool)inputCallable, "Input callable is invokable"))
      return false;

    Callable<int(int)> callable(nullptr);
    if (!check(!callable, "New callable instance should not be invokable"))
      return false;

    callable = inputCallable;
    if (!check((bool)callable, "Callable should now be invokable"))
      return false;
    if (!check(callable == inputCallable, "Callable should be a copy of input callable"))
      return false;

    if (!check(callable(Forward<int>(param)) == result, "Callable should've returned %d", result))
      return false;

    return true;
  }

  bool testLambdaBasic() {
    int egg = 20;
    auto fn = [&egg] (int x) {
      return egg + x + 1;
    };
    Lambda<int(int)> ptr = fn;
    if (!check(ptr(10) == 31, "capture local variable in Lambda"))
      return false;

    ptr = [] (int x) -> int {
      return x + 15;
    };
    if (!check(ptr(7) == 22, "assign new function to Lambda"))
      return false;

    ptr = test_old_fn;
    if (!check(ptr(10) == 109, "assign static function to Lambda"))
      return false;

    CallableObj obj;
    ptr = obj;
    if (!check(ptr(66) == 100, "assign callable obj to Lambda"))
      return false;

    Lambda<unsigned(MoveObj&& obj)> ptr2 = [] (MoveObj&& obj) -> unsigned {
      MoveObj other(Move(obj));
      return other.count();
    };

    MoveObj moveObj;
    if (!check(ptr2(Move(moveObj)) == 1, "moved Lambda arguments"))
      return false;

    return true;
  }

  bool testFunctionPointer() {
    FunctionPointer<int(int)> ptr = test_old_fn;

    if (!check(ptr(1) == 100, "FunctionPointer called static function"))
      return false;

    auto fn = [](int x) -> int {
      return x + 2;
    };
    ptr = &fn;

    if (!check(ptr(10) == 12, "FunctionPointer called lambda"))
      return false;

    CallableObj obj;
    ptr = &obj;
    if (!check(ptr(7) == 41, "FunctionPointer called callable object"))
      return false;

    ptr.assign(&obj, &CallableObj::member_function);
    if (!check(ptr(5) == 30, "FunctionPointer called member function"))
      return false;

    return true;
  }

  bool testMoveUncopyable() {
#if defined(KE_CXX_HAS_GENERIC_LAMBDA_CAPTURES)
    Vector<int> v;

    auto lambda = [v = Move(v)]() -> size_t {
      return v.length();
    };

    v.append(10);
    Lambda<size_t()> f = Move(lambda);

    if (!check(f() == 0, "f() should have returned 0"))
      return false;
#endif

    return true;
  }

  bool Run() override
  {
    if (!testCallable(Lambda<int(int)>([](int input) { return (input * input) * 10; }), 4, 160))
      return false;
    if (!testLambdaBasic())
      return false;
    if (!testFunctionPointer())
      return false;
    if (!testMoveUncopyable())
      return false;
    return true;
  };
} sTestCallable;
