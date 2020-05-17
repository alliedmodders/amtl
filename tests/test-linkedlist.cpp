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

#include <amtl/am-linkedlist.h>
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

  private:
    MovingThing(const MovingThing& other) = delete;
    bool moved_;
};

} // anonymous namespace

TEST(LinkedList, Ints) {
    LinkedList<int> list;

    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.begin(), list.end());

    list.append(5);
    list.append(7);
    list.append(9);
    EXPECT_TRUE(!list.empty());
    EXPECT_EQ(list.length(), (size_t)3);

    LinkedList<int>::iterator iter = list.begin();
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 5);
    iter++;
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 7);
    iter++;
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 9);
    iter++;
    EXPECT_EQ(iter, list.end());

    iter = list.begin();
    iter++;
    iter++;
    iter = list.erase(iter);
    EXPECT_EQ(iter, list.end());
    list.append(11);

    iter = list.begin();
    iter = list.erase(iter);
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 7);
    iter++;
    EXPECT_EQ(*iter, 11);
    EXPECT_EQ(list.length(), (size_t)2);

    list.prepend(1);
    iter = list.begin();
    EXPECT_EQ(*iter, 1);
    iter++;
    EXPECT_EQ(*iter, 7);
    EXPECT_EQ(list.length(), (size_t)3);

    list.clear();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.begin(), list.end());

    list.append(1);
    list.append(2);
    list.append(3);
    list.append(4);
    list.append(5);
    list.append(5);
    iter = list.find(3);
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 3);
    list.remove(3);
    iter = list.find(3);
    EXPECT_EQ(iter, list.end());
    iter = list.find(5);
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 5);
    list.remove(5);
    iter = list.find(5);
    EXPECT_TRUE(iter != list.end());
    EXPECT_EQ(*iter, 5);
    list.remove(5);
    list.remove(5);
    iter = list.find(5);
    EXPECT_EQ(iter, list.end());
}

TEST(LinkedList, Destructors) {
    ResetGlobals();
    {
        LinkedList<BasicThing> list;
        list.append(BasicThing());
        list.append(BasicThing());
        list.append(BasicThing());
    }
    EXPECT_EQ(sCtors, 3);
    EXPECT_EQ(sCopyCtors, 3);
    EXPECT_EQ(sDtors, 6);
}

TEST(LinkedList, Moving) {
    ResetGlobals();
    {
        LinkedList<MovingThing> list;
        MovingThing a, b, c;
        list.append(std::move(a));
        list.append(std::move(b));
        list.append(std::move(c));
    }

    EXPECT_EQ(sCtors, 3);
    EXPECT_EQ(sMovingCtors, 3);
    EXPECT_EQ(sMovedDtors, 3);
    EXPECT_EQ(sDtors, 6);
}

TEST(LinkedList, FallibleMalloc) {
    LinkedList<int, FallibleMalloc> list;
    list.append(5);
    list.append(6);
    list.allocPolicy().setOutOfMemory(true);
    EXPECT_FALSE(list.append(7));
    list.allocPolicy().setOutOfMemory(false);
    ASSERT_TRUE(list.append(8));
    EXPECT_EQ(list.allocPolicy().ooms(), (size_t)1);
}
