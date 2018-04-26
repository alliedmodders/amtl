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

  MoveObj()
  {
    sNumMoves = 0;
  }
  MoveObj(MoveObj&& other) {
    sNumMoves++;
  }

  unsigned count() const {
    return sNumMoves;
  }

 private:
  MoveObj(const MoveObj& other);
  void operator =(const MoveObj& other);
};
unsigned MoveObj::sNumMoves = 0;

class CallableObj
{
 public:
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

  bool testLambdaBasic() {
    int egg = 20;
    auto fn = [&egg](int x) -> int {
      return egg + x + 1;
    };
    
    Lambda<int(int)> ptr(fn);
    if (!check(ptr(10) == 31, "capture local variable in Lambda"))
      return false;

    ptr = [](int x) -> int {
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

    Lambda<unsigned(MoveObj&& obj)> ptr2 = [](MoveObj&& obj) -> unsigned {
      MoveObj other(ke::Move(obj));
      return other.count();
    };

    MoveObj moveObj;
    if (!check(ptr2(ke::Move(moveObj)) == 1, "moved Lambda arguments"))
      return false;

    return true;
  }

  bool testInlineStorage() {
    Lambda<int()> ptr = []() -> int {
      return 10;
    };

    if (!check(ptr.usingInlineStorage(), "small lambda should be using inline storage"))
      return false;
    if (!check(ptr() == 10, "small lambda should have correct function"))
      return false;

    static size_t dtors = 0;
    struct CallDtorObj {
      ~CallDtorObj() {
        dtors++;
      }
    };

    struct {
      int a;
      void* b, *c, *d, *e, *f, *g;
      void* h, *j, *k, *m, *n, *o, *p;
    } huge_struct = { 20 };
    CallDtorObj test_dtor;
    ptr = [huge_struct, test_dtor]() -> int {
      return huge_struct.a;
    };
    if (!check(!ptr.usingInlineStorage(), "huge lambda should not be using inline storage"))
      return false;
    if (!check(ptr() == 20, "huge lambda should have correct function"))
      return false;

    ptr = nullptr;
    if (!check(dtors == 2, "got 2 destructors"))
      return false;

    return true;
  }

  bool testMove() {
    static size_t ctors = 0;
    static size_t copyctors = 0;
    static size_t movectors = 0;
    static size_t dtors = 0;
    struct CallDtorObj {
      CallDtorObj() {
        ctors++;
      }
      CallDtorObj(const CallDtorObj& other) {
        copyctors++;
      }
      CallDtorObj(CallDtorObj&& other) {
        movectors++;
      }
      ~CallDtorObj() {
        dtors++;
      }
    };

    CallDtorObj test_dtor;
    Lambda<void()> ptr = [test_dtor] {
    };

    if (!check(dtors == 1, "got 1 destructor"))
      return false;

    ctors = 0;
    copyctors = 0;
    movectors = 0;
    dtors = 0;

    Lambda<void()> ptr2 = ptr;
    if (!check(ctors == 0, "no constructors called"))
      return false;
    if (!check(copyctors == 1, "got one copy constructor"))
      return false;
    if (!check(movectors == 0, "no move constructors called"))
      return false;
    if (!check(dtors == 0, "no destructors called"))
      return false;

    copyctors = 0;

    Lambda<void()> ptr3 = ke::Move(ptr2);
    if (!check(ctors == 0, "no constructors called"))
      return false;
    if (!check(copyctors == 0, "no copy constructors called"))
      return false;
    if (!check(movectors == 0, "no move constructors called"))
      return false;
    if (!check(dtors == 0, "no destructors called"))
      return false;

    copyctors = 0;

    auto fn = [test_dtor]{};
    Lambda<void()> ptr4 = ke::Move(fn);
    if (!check(ctors == 0, "no constructors called"))
      return false;
#if !defined(_MSC_VER) || (_MSC_VER >= 1900)
    if (!check(copyctors == 1, "no copy constructors called"))
      return false;
    if (!check(movectors == 1, "no move constructors called"))
      return false;
#else
    // Older Microsoft compilers do not implement move semantics for lambda
    // types, unfortunately.
    if (!check(copyctors == 2, "no copy constructors called"))
      return false;
    if (!check(movectors == 0, "no move constructors called"))
      return false;
#endif
    if (!check(dtors == 0, "no destructors called"))
      return false;

    return true;
  }

  bool testFuncPtr() {
    FuncPtr<int(int)> ptr = test_old_fn;

    if (!check(ptr(1) == 100, "FuncPtr called static function"))
      return false;

    auto fn = [](int x) -> int {
      return x + 2;
    };
    ptr = &fn;

    if (!check(ptr(10) == 12, "FuncPtr called lambda"))
      return false;

    CallableObj obj;
    ptr = &obj;
    if (!check(ptr(7) == 41, "FuncPtr called callable object"))
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
    Function<size_t()> f = ke::Move(lambda);

    if (!check(f() == 0, "f() should have returned 0"))
      return false;
#endif

    return true;
  }

  bool Run() override
  {
    if (!testLambdaBasic())
      return false;
    if (!testInlineStorage())
      return false;
    if (!testMove())
      return false;
    if (!testFuncPtr())
      return false;
    if (!testMoveUncopyable())
      return false;
    return true;
  };
} sTestCallable;
