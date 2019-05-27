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

#include <amtl/am-uniqueptr.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

static int sBrahCtors = 0;
static int sBrahDtors = 0;
struct Brah {
    Brah() {
        sBrahCtors++;
    }
    ~Brah() {
        sBrahDtors++;
    }
};

TEST(UniquePtr, Single) {
    UniquePtr<int> five = MakeUnique<int>(5);
    EXPECT_EQ(*five.get(), 5);

    five = nullptr;
    EXPECT_FALSE(five);

    {
        UniquePtr<Brah> blah = MakeUnique<Brah>();
        EXPECT_EQ(sBrahCtors, 1);
    }
    EXPECT_EQ(sBrahDtors, 1);

    sBrahCtors = 0;
    sBrahDtors = 0;
    {
        UniquePtr<Brah[]> blah = MakeUnique<Brah[]>(20);
        EXPECT_EQ(sBrahCtors, 20);
    }
    EXPECT_EQ(sBrahDtors, 20);
}
