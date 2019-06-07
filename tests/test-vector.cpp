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

#include <amtl/am-refcounting.h>
#include <amtl/am-vector.h>
#include <assert.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

static int sCtors = 0;
static int sCopyCtors = 0;
static int sMovingCtors = 0;
static int sMovedDtors = 0;
static int sDtors = 0;

namespace {

static void
ResetGlobals()
{
    sCtors = 0;
    sCopyCtors = 0;
    sMovingCtors = 0;
    sMovedDtors = 0;
    sDtors = 0;
}

class BasicThing
{
  public:
    BasicThing() {
        sCtors++;
    }
    BasicThing(const BasicThing& other) {
        sCopyCtors++;
    }
    ~BasicThing() {
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
    MovingThing(MovingThing&& other) {
        assert(!other.moved_);
        sMovingCtors++;
        moved_ = false;
        other.moved_ = true;
    }
    ~MovingThing() {
        sDtors++;
        if (moved_)
            sMovedDtors++;
    }
    MovingThing& operator =(MovingThing&& other) {
        assert(!other.moved_);
        sCopyCtors++;
        moved_ = false;
        other.moved_ = true;
        return *this;
    }
    bool moved() const {
        return moved_;
    }

  private:
    MovingThing(const MovingThing& other) = delete;
    bool moved_;
};

} // anonymous namespace

TEST(Vector, Ints) {
    Vector<int> vector;

    EXPECT_TRUE(vector.empty());
    EXPECT_EQ(vector.length(), (size_t)0);

    vector.append(1);
    vector.append(2);
    vector.append(3);
    vector.append(4);
    vector.append(5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(vector[i], i + 1);
    }
    EXPECT_EQ(vector.length(), (size_t)5);
    EXPECT_FALSE(vector.empty());

    EXPECT_EQ(vector.popCopy(), 5);
    EXPECT_EQ(vector.popCopy(), 4);
    EXPECT_EQ(vector.length(), (size_t)3);

    vector.insert(0, 88);
    vector.insert(0, 99);
    vector.insert(4, 111);
    EXPECT_EQ(vector[0], 99);
    EXPECT_EQ(vector[1], 88);
    EXPECT_EQ(vector[2], 1);
    EXPECT_EQ(vector[3], 2);
    EXPECT_EQ(vector[4], 111);
    EXPECT_EQ(vector[5], 3);
    EXPECT_EQ(vector.length(), (size_t)6);

    vector.remove(5);
    EXPECT_EQ(vector[4], 111);
    vector.remove(0);
    EXPECT_EQ(vector[0], 88);
    EXPECT_EQ(vector[3], 111);
    EXPECT_EQ(vector.length(), (size_t)4);

    while (!vector.empty())
        vector.pop();

    EXPECT_TRUE(vector.empty());
    EXPECT_TRUE(vector.ensure(128));
    EXPECT_TRUE(vector.empty());

    for (int i = 0; i < 128; i++)
        vector.append(i);
    EXPECT_EQ(vector.length(), (size_t)128);

    vector.clear();
    EXPECT_TRUE(vector.empty());
}

TEST(Vector, Destructors) {
    ResetGlobals();

    {
        Vector<BasicThing> vector;
        vector.append(BasicThing());
        vector.append(BasicThing());
        vector.append(BasicThing());
    }
    EXPECT_EQ(sCtors, 3);
    EXPECT_EQ(sCopyCtors, 3);
    EXPECT_EQ(sDtors, 6);

    sDtors = 0;
    {
        Vector<BasicThing> vector;
        vector.append(BasicThing());
        vector.clear();
    }
    EXPECT_EQ(sDtors, 2);
}

TEST(Vector, Moving) {
    ResetGlobals();
    {
        Vector<MovingThing> vector;
        MovingThing a, b, c;
        vector.append(ke::Move(a));
        vector.append(ke::Move(b));
        vector.append(ke::Move(c));
    }

    EXPECT_EQ(sCtors, 3);
    EXPECT_EQ(sMovingCtors, 3);
    EXPECT_EQ(sMovedDtors, 3);
    EXPECT_EQ(sDtors, 6);

    sCtors = 0;
    sMovingCtors = 0;
    sMovedDtors = 0;
    sDtors = 0;

    Vector<int> v1;
    v1.append(10);
    Vector<int> v2(Move(v1));
    EXPECT_EQ(v2.length(), (size_t)1);
    EXPECT_EQ(v1.length(), (size_t)0);
}

TEST(Vector, FallibleMalloc) {
    Vector<int, FallibleMalloc> vector;
    vector.allocPolicy().setOutOfMemory(true);
    EXPECT_FALSE(vector.append(7));
    vector.allocPolicy().setOutOfMemory(false);
    EXPECT_TRUE(vector.append(8));
    EXPECT_EQ(vector.allocPolicy().ooms(), (size_t)1);
}

TEST(Vector, MoveDuringInsert) {
    Vector<MovingThing> vector;
    for (size_t i = 1; i <= 8; i++) {
        MovingThing x;
        vector.append(Move(x));
    }
    {
        MovingThing x;
        vector.insert(0, Move(x));
    }
    for (size_t i = 0; i < vector.length(); i++) {
        EXPECT_FALSE(vector[i].moved());
    }
}

TEST(Vector, Resize) {
    Vector<bool> vector;
    vector.append(true);
    vector.resize(100);
    EXPECT_EQ(vector.length(), (size_t)100);
    EXPECT_TRUE(vector[0]);
    for (size_t i = 1; i < 100; i++) {
        EXPECT_FALSE(vector[i]);
    }
    vector.resize(1);
    ASSERT_EQ(vector.length(), (size_t)1);
    for (size_t i = 0; i < 1; i++) {
        EXPECT_TRUE(vector[i]);
    }
}

TEST(Vector, Remove) {
    class HeldThing
    {
      public:
        HeldThing()
         : refcount(0)
        {}
        virtual ~HeldThing() {}

        void AddRef() {
            refcount++;
        }
        void Release() {
            if (--refcount == 0)
                delete this;
        }

        uintptr_t refcount;
    };

    class WrappedThing
    {
      public:
        WrappedThing(HeldThing* thing)
         : thing(thing)
        {}
        WrappedThing(WrappedThing&& other)
         : thing(ke::Move(other.thing))
        {}
        WrappedThing& operator =(WrappedThing&& other) {
            thing = ke::Move(other.thing);
            return *this;
        }

        ke::RefPtr<HeldThing> thing;

      private:
        WrappedThing(const WrappedThing& other) = delete;
        void operator =(const WrappedThing& other) = delete;
    };

    ke::RefPtr<HeldThing> thing1(new HeldThing);
    ke::RefPtr<HeldThing> thing2(new HeldThing);

    Vector<WrappedThing> things;
    things.append(WrappedThing(thing1));
    things.append(WrappedThing(thing2));

    EXPECT_EQ(thing1->refcount, (size_t)2);
    EXPECT_EQ(thing2->refcount, (size_t)2);

    things.remove(0);

    EXPECT_EQ(thing1->refcount, (size_t)1);
    EXPECT_EQ(thing2->refcount, (size_t)2);

    things.remove(0);

    EXPECT_EQ(thing1->refcount, (size_t)1);
    EXPECT_EQ(thing2->refcount, (size_t)1);
}
