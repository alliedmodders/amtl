// vim: set sts=4 ts=8 sw=4 tw=99 et:
//
// Copyright (C) 2021 AlliedModders LLC
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

#include <amtl/am-raii.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

TEST(ScopeGuard, Invoke)
{
    int invokes = 0;

    {
        auto guard1 = MakeScopeGuard([&]() -> void {
            invokes++;
        });

        {
            auto guard2 = MakeScopeGuard([&]() -> void {
                invokes++;
            });
        }
        ASSERT_EQ(invokes, 1);
    }
    ASSERT_EQ(invokes, 2);
}

TEST(ScopeGuard, Cancel)
{
    int invokes = 0;

    {
        auto guard1 = MakeScopeGuard([&]() -> void {
            invokes++;
        });

        guard1.cancel();
    }
    ASSERT_EQ(invokes, 0);
}
