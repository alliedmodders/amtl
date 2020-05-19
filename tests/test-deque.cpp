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
#include <amtl/am-deque.h>
#include <assert.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

TEST(Deque, Basic) {
    std::deque<int> dq;

    for (int i = 0; i < 4; i++) {
        dq.push_back(i);
        ASSERT_EQ(dq.size(), size_t(i + 1));
    }
    for (int i = 0; i < 4; i++) {
        dq.push_front(i + 4);
        ASSERT_EQ(dq.size(), size_t(i + 5));
    }

    EXPECT_EQ(dq.back(), 3);
    EXPECT_EQ(dq.front(), 7);
    dq.pop_back();
    dq.pop_front();
    EXPECT_EQ(PopFront(&dq), 6);
    EXPECT_EQ(PopBack(&dq), 2);

    EXPECT_EQ(dq.size(), (size_t)4);

    while (!dq.empty())
        dq.pop_back();
    EXPECT_EQ(dq.size(), (size_t)0);
}

TEST(Deque, PrependEmpty) {
    std::deque<int> dq;

    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0)
            dq.push_front(i);
        else
            dq.push_back(i);
        EXPECT_EQ(dq.size(), size_t(i + 1));
    }

    while (!dq.empty())
        dq.pop_front();
    EXPECT_EQ(dq.size(), (size_t)0);
}

TEST(Deque, Resize) {
    std::deque<int> dq;
    for (int i = 0; i < 387; i++)
        dq.push_front(i);
    for (int i = 0; i < 293; i++)
        dq.push_back(i);

    ASSERT_EQ(dq.size(), size_t(293 + 387));

    for (int i = 292; i >= 0; i--) {
        EXPECT_EQ(PopBack(&dq), i);
    }
    for (int i = 386; i >= 0; i--) {
        EXPECT_EQ(PopFront(&dq), i);
    }

    // Check that we can still add.
    dq.push_back(5);
    ASSERT_EQ(PopFront(&dq), 5);
    dq.push_back(6);
    ASSERT_EQ(PopBack(&dq), 6);

    dq.push_front(7);
    ASSERT_EQ(PopBack(&dq), 7);
    dq.push_front(8);
    ASSERT_EQ(PopFront(&dq), 8);
}

TEST(Deque, Move) {
    std::deque<int> dq1;

    dq1.push_back(10);

    {
        std::deque<int> dq2 = std::move(dq1);
        ASSERT_EQ(dq2.size(), (size_t)1);
        ASSERT_EQ(PopFront(&dq2), 10);
    }

    ASSERT_EQ(dq1.size(), (size_t)0);

    // Append so we can make sure that it's not holding a deleted pointer.
    dq1.push_back(11);
}
