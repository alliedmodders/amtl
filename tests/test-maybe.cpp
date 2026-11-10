// vim: set sts=4 ts=8 sw=4 tw=99 et:
//
// Copyright (C) 2026 AlliedModders LLC
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
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.

#include <amtl/am-maybe.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

namespace {

// Counts live instances and records the order of destructions, like the RAII
// scopes in SourcePawn's code generator that push state on construction and
// pop it on destruction.
class LiveCounter {
  public:
    explicit LiveCounter(int id)
      : id_(id)
    {
        live_++;
    }
    LiveCounter(const LiveCounter& other)
      : id_(other.id_)
    {
        live_++;
    }

    ~LiveCounter() {
        live_--;
        destroyed_[destroyed_count_++] = id_;
    }

    int id() const { return id_; }

    static int live() { return live_; }
    static int destroyed_count() { return destroyed_count_; }
    static int destroyed(int index) { return destroyed_[index]; }

    static void Reset() {
        live_ = 0;
        destroyed_count_ = 0;
    }

  private:
    int id_;
    static int live_;
    static int destroyed_[16];
    static int destroyed_count_;
};

int LiveCounter::live_ = 0;
int LiveCounter::destroyed_[16] = {};
int LiveCounter::destroyed_count_ = 0;

} // namespace

TEST(Maybe, AssignEmptyMoveDestroysOld)
{
    LiveCounter::Reset();

    Maybe<LiveCounter> m;
    m.init(1);
    ASSERT_EQ(LiveCounter::live(), 1);

    // This is the pattern used by SourcePawn's EmitForStmt, which relies on
    // the scope object being destroyed to pop state.
    m = Maybe<LiveCounter>();

    EXPECT_EQ(LiveCounter::live(), 0);
    ASSERT_EQ(LiveCounter::destroyed_count(), 1);
    EXPECT_EQ(LiveCounter::destroyed(0), 1);
    EXPECT_FALSE(m.isValid());
}

TEST(Maybe, AssignEmptyCopyDestroysOld)
{
    LiveCounter::Reset();

    Maybe<LiveCounter> m;
    m.init(1);
    ASSERT_EQ(LiveCounter::live(), 1);

    const Maybe<LiveCounter> empty;
    m = empty;

    EXPECT_EQ(LiveCounter::live(), 0);
    ASSERT_EQ(LiveCounter::destroyed_count(), 1);
    EXPECT_EQ(LiveCounter::destroyed(0), 1);
    EXPECT_FALSE(m.isValid());
}

TEST(Maybe, AssignValidMoveDestroysOld)
{
    LiveCounter::Reset();

    Maybe<LiveCounter> m;
    m.init(1);
    ASSERT_EQ(LiveCounter::live(), 1);

    Maybe<LiveCounter> other;
    other.init(2);
    ASSERT_EQ(LiveCounter::live(), 2);

    m = std::move(other);

    // The old value must be destroyed, and the moved-from value must be
    // destroyed as well once its contents have been stolen. Only the new
    // value in |m| should remain live.
    EXPECT_EQ(LiveCounter::live(), 1);
    ASSERT_EQ(LiveCounter::destroyed_count(), 2);
    EXPECT_EQ(LiveCounter::destroyed(0), 1);
    EXPECT_EQ(LiveCounter::destroyed(1), 2);
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ((*m).id(), 2);
    EXPECT_FALSE(other.isValid());
}

TEST(Maybe, AssignValidCopyDestroysOld)
{
    LiveCounter::Reset();

    Maybe<LiveCounter> m;
    m.init(1);
    ASSERT_EQ(LiveCounter::live(), 1);

    Maybe<LiveCounter> other;
    other.init(2);
    ASSERT_EQ(LiveCounter::live(), 2);

    m = other;

    // The old value must have been destroyed exactly once.
    EXPECT_EQ(LiveCounter::live(), 2);
    ASSERT_EQ(LiveCounter::destroyed_count(), 1);
    EXPECT_EQ(LiveCounter::destroyed(0), 1);
    EXPECT_TRUE(m.isValid());
    EXPECT_EQ((*m).id(), 2);
    EXPECT_TRUE(other.isValid());
}

TEST(Maybe, InitOverValidDestroysOld)
{
    LiveCounter::Reset();

    Maybe<LiveCounter> m;
    m.init(1);
    m.init(2);

    EXPECT_EQ(LiveCounter::live(), 1);
    ASSERT_EQ(LiveCounter::destroyed_count(), 1);
    EXPECT_EQ(LiveCounter::destroyed(0), 1);
    EXPECT_EQ((*m).id(), 2);

    m = Maybe<LiveCounter>();
    EXPECT_EQ(LiveCounter::live(), 0);
    ASSERT_EQ(LiveCounter::destroyed_count(), 2);
    EXPECT_EQ(LiveCounter::destroyed(1), 2);
}
