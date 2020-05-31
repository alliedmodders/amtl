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
    std::vector<int> vector;

    EXPECT_TRUE(vector.empty());
    EXPECT_EQ(vector.size(), (size_t)0);

    vector.emplace_back(1);
    vector.emplace_back(2);
    vector.emplace_back(3);
    vector.emplace_back(4);
    vector.emplace_back(5);
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(vector[i], i + 1);
    }
    EXPECT_EQ(vector.size(), (size_t)5);
    EXPECT_FALSE(vector.empty());

    EXPECT_EQ(PopBack(&vector), 5);
    EXPECT_EQ(PopBack(&vector), 4);
    EXPECT_EQ(vector.size(), (size_t)3);

    ke::InsertAt(&vector, 0, 88);
    ke::InsertAt(&vector, 0, 99);
    ke::InsertAt(&vector, 4, 111);
    EXPECT_EQ(vector[0], 99);
    EXPECT_EQ(vector[1], 88);
    EXPECT_EQ(vector[2], 1);
    EXPECT_EQ(vector[3], 2);
    EXPECT_EQ(vector[4], 111);
    EXPECT_EQ(vector[5], 3);
    EXPECT_EQ(vector.size(), (size_t)6);

    ke::RemoveAt(&vector, 5);
    EXPECT_EQ(vector[4], 111);
    ke::RemoveAt(&vector, 0);
    EXPECT_EQ(vector[0], 88);
    EXPECT_EQ(vector[3], 111);
    EXPECT_EQ(vector.size(), (size_t)4);

    while (!vector.empty())
        vector.pop_back();

    EXPECT_TRUE(vector.empty());

    for (int i = 0; i < 128; i++)
        vector.emplace_back(i);
    EXPECT_EQ(vector.size(), (size_t)128);

    vector.clear();
    EXPECT_TRUE(vector.empty());
}

TEST(Vector, Destructors) {
    ResetGlobals();

    {
        std::vector<BasicThing> vector;
        vector.emplace_back();
        vector.emplace_back();
        vector.emplace_back();
    }
    EXPECT_EQ(sCtors, 3);
    EXPECT_EQ(sCopyCtors, 3);
    EXPECT_EQ(sDtors, 6);

    sDtors = 0;
    {
        std::vector<BasicThing> vector;
        vector.emplace_back(BasicThing());
        vector.clear();
    }
    EXPECT_EQ(sDtors, 2);
}

TEST(Vector, Moving) {
    ResetGlobals();
    {
        std::vector<MovingThing> vector;
        MovingThing a, b, c;
        vector.emplace_back(std::move(a));
        vector.emplace_back(std::move(b));
        vector.emplace_back(std::move(c));
    }

    EXPECT_EQ(sCtors, 3);
    EXPECT_GE(sMovingCtors, 3);
    EXPECT_EQ(sMovedDtors, sMovingCtors);
    EXPECT_GT(sDtors, sMovingCtors);

    sCtors = 0;
    sMovingCtors = 0;
    sMovedDtors = 0;
    sDtors = 0;

    std::vector<int> v1;
    v1.emplace_back(10);
    std::vector<int> v2(std::move(v1));
    EXPECT_EQ(v2.size(), (size_t)1);
    EXPECT_EQ(v1.size(), (size_t)0);
}

TEST(Vector, MoveDuringInsert) {
    std::vector<MovingThing> vector;
    for (size_t i = 1; i <= 8; i++) {
        MovingThing x;
        vector.emplace_back(std::move(x));
    }
    {
        MovingThing x;
        ke::InsertAt(&vector, 0, std::move(x));
    }
    for (size_t i = 0; i < vector.size(); i++) {
        EXPECT_FALSE(vector[i].moved());
    }
}

TEST(Vector, Resize) {
    std::vector<bool> vector;
    vector.emplace_back(true);
    vector.resize(100);
    EXPECT_EQ(vector.size(), (size_t)100);
    EXPECT_TRUE(vector[0]);
    for (size_t i = 1; i < 100; i++) {
        EXPECT_FALSE(vector[i]);
    }
    vector.resize(1);
    ASSERT_EQ(vector.size(), (size_t)1);
    for (size_t i = 0; i < 1; i++) {
        EXPECT_TRUE(vector[i]);
    }
}

TEST(Vector, InsertAtEnd) {
    std::vector<int> vector;
    ke::InsertAt(&vector, 0, 555);
    ASSERT_EQ(vector.size(), (size_t)1);
    EXPECT_EQ(vector[0], 555);
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
         : thing(std::move(other.thing))
        {}
        WrappedThing& operator =(WrappedThing&& other) {
            thing = std::move(other.thing);
            return *this;
        }

        ke::RefPtr<HeldThing> thing;

      private:
        WrappedThing(const WrappedThing& other) = delete;
        void operator =(const WrappedThing& other) = delete;
    };

    ke::RefPtr<HeldThing> thing1(new HeldThing);
    ke::RefPtr<HeldThing> thing2(new HeldThing);

    std::vector<WrappedThing> things;
    things.emplace_back(WrappedThing(thing1));
    things.emplace_back(WrappedThing(thing2));

    EXPECT_EQ(thing1->refcount, (size_t)2);
    EXPECT_EQ(thing2->refcount, (size_t)2);

    ke::RemoveAt(&things, 0);

    EXPECT_EQ(thing1->refcount, (size_t)1);
    EXPECT_EQ(thing2->refcount, (size_t)2);

    ke::RemoveAt(&things, 0);

    EXPECT_EQ(thing1->refcount, (size_t)1);
    EXPECT_EQ(thing2->refcount, (size_t)1);
}
