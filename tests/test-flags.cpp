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

#include <amtl/am-flags.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include "runner.h"

using namespace ke;

enum class Scoped : uint32_t {
    Flag0 = (1 << 0),
    Flag1 = (1 << 1),
    Flag2 = (1 << 2),
    Flag3 = (1 << 3)
};
KE_DEFINE_ENUM_OPERATORS(Scoped)

TEST(Flags, Basic) {
    Flags<Scoped> flags;
    ASSERT_TRUE(!flags);
    ASSERT_FALSE(flags);

    flags += Scoped::Flag0;
    ASSERT_TRUE((bool)flags);

    flags += Scoped::Flag1;

    Flags<Scoped> other(Scoped::Flag1 | Scoped::Flag2);
    ASSERT_NE(flags, other);
    ASSERT_EQ(other.bits(), (uint32_t)0x6);

    flags -= other;
    ASSERT_EQ(flags, Scoped::Flag0);

    uint32_t value = flags.bits();
    ASSERT_EQ(value, (uint32_t)0x1);

    flags |= other;
    ASSERT_EQ(flags.bits(), (uint32_t)0x7);

    flags &= other;
    ASSERT_EQ(flags.bits(), (uint32_t)0x6);
}
