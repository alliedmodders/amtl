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

#include <amtl/am-function.h>
#include <amtl/am-vector.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

static int
test_old_fn(int x)
{
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
    MoveObj(const MoveObj& other);
    void operator =(const MoveObj& other);
};
unsigned MoveObj::sNumMoves = 0;

class CallableObj
{
  public:
    int operator ()(int x) const {
        return x + 34;
    }
};

TEST(Callable, BasicFunction) {
    int egg = 20;
    auto fn = [&egg](int x) -> int { return egg + x + 1; };

    Function<int(int)> ptr(fn);
    EXPECT_EQ(ptr(10), 31);

    ptr = [](int x) -> int { return x + 15; };
    EXPECT_EQ(ptr(7), 22);

    ptr = test_old_fn;
    EXPECT_EQ(ptr(10), 109);

    CallableObj obj;
    ptr = obj;
    EXPECT_EQ(ptr(66), 100);

    Function<unsigned(MoveObj && obj)> ptr2 = [](MoveObj&& obj) -> unsigned {
        MoveObj other(std::move(obj));
        return other.count();
    };

    MoveObj moveObj;
    EXPECT_EQ(ptr2(std::move(moveObj)), (unsigned)1);
}

TEST(Callable, InlineStorage) {
    Function<int()> ptr = []() -> int { return 10; };

    EXPECT_TRUE(ptr.usingInlineStorage());
    EXPECT_EQ(ptr(), 10);

    static size_t dtors = 0;
    struct CallDtorObj {
        ~CallDtorObj() {
            dtors++;
        }
    };

    struct {
        int a;
        void *b, *c, *d, *e, *f, *g;
        void *h, *j, *k, *m, *n, *o, *p;
    } huge_struct = {20};
    CallDtorObj test_dtor;
    ptr = [huge_struct, test_dtor]() -> int { return huge_struct.a; };
    EXPECT_FALSE(ptr.usingInlineStorage());
    EXPECT_EQ(ptr(), 20);

    ptr = nullptr;
    EXPECT_EQ(dtors, (size_t)2);
}

TEST(Callable, Move) {
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
    Function<void()> ptr = [test_dtor] {};

    EXPECT_EQ(dtors, (size_t)1);

    ctors = 0;
    copyctors = 0;
    movectors = 0;
    dtors = 0;

    Function<void()> ptr2 = ptr;
    EXPECT_EQ(ctors, (size_t)0);
    EXPECT_EQ(copyctors, (size_t)1);
    EXPECT_EQ(movectors, (size_t)0);
    EXPECT_EQ(dtors, (size_t)0);

    copyctors = 0;

    Function<void()> ptr3 = std::move(ptr2);
    EXPECT_EQ(ctors, (size_t)0);
    EXPECT_EQ(copyctors, (size_t)0);
    EXPECT_EQ(movectors, (size_t)0);
    EXPECT_EQ(dtors, (size_t)0);

    copyctors = 0;

    auto fn = [test_dtor] {};
    Function<void()> ptr4 = std::move(fn);
    EXPECT_EQ(ctors, (size_t)0);
    EXPECT_EQ(copyctors, (size_t)1);
    EXPECT_EQ(movectors, (size_t)1);
    EXPECT_EQ(dtors, (size_t)0);
}
