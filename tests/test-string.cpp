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

#include <amtl/am-string.h>
#include <gtest/gtest.h>
#include <limits.h>
#include "runner.h"

using namespace ke;

TEST(String, Sprintf) {
    int64_t x = 1234;
    char buffer[256];
    SafeSprintf(buffer, sizeof(buffer), "%" KE_FMT_I64, x);
    ASSERT_EQ(strcmp(buffer, "1234"), 0);
}

TEST(String, Allocating) {
    int a = INT_MAX;
    const char* value = "Hello this is a test.";
    const char* expect = "A: 2147483647 B: Hello this is a test.";
    UniquePtr<char[]> ptr = Sprintf("A: %d B: %s", a, value);
    EXPECT_EQ(strcmp(ptr.get(), expect), 0);

    UniquePtr<AString> str = AString::Sprintf("A: %d B: %s", a, value);
    EXPECT_EQ(str->compare(expect), 0);
}

TEST(String, Split) {
    Vector<AString> out = Split("     ", " ");
    EXPECT_EQ(out.length(), (size_t)6);
    for (size_t i = 0; i < out.length(); i++) {
        EXPECT_EQ(out[i].length(), (size_t)0);
    }

    out = Split("egg", " ");
    EXPECT_EQ(out.length(), (size_t)1);
    EXPECT_EQ(out[0].compare("egg"), 0);

    out = Split("", "egg");
    EXPECT_EQ(out.length(), (size_t)0);

    out = Split("xaba", "a");
    EXPECT_EQ(out.length(), (size_t)3);
    EXPECT_EQ(out[0].compare("x"), 0);
    EXPECT_EQ(out[1].compare("b"), 0);
    EXPECT_EQ(out[2].compare(""), 0);

    out = Split("egg ham", " ");
    EXPECT_EQ(out.length(), (size_t)2);
    EXPECT_EQ(out[0].compare("egg"), 0);
    EXPECT_EQ(out[1].compare("ham"), 0);
}

TEST(String, Join) {
    Vector<AString> in;

    AString result = Join(in, "x");
    EXPECT_EQ(result.compare(""), 0);

    in.append("abc");
    result = Join(in, "x");
    EXPECT_EQ(result.compare("abc"), 0);

    in.append("xyz");
    result = Join(in, "T");
    EXPECT_EQ(result.compare("abcTxyz"), 0);

    in.append("def");
    result = Join(in, "");
    EXPECT_EQ(result.compare("abcxyzdef"), 0);
}

TEST(String, Case) {
    AString str("samPle1.com");
    str = str.uppercase();
    EXPECT_EQ(str.compare("SAMPLE1.COM"), 0);

    str = str.lowercase();
    EXPECT_EQ(str.compare("sample1.com"), 0);

    str = AString();
    str = str.lowercase();
    EXPECT_EQ(str.compare(""), 0);
}

TEST(String, StrCpy) {
    char buffer[6];

    SafeStrcpy(buffer, sizeof(buffer), "hello");
    EXPECT_EQ(strcmp(buffer, "hello"), 0);

    SafeStrcpy(buffer, sizeof(buffer), "hello, crab");
    EXPECT_EQ(strcmp(buffer, "hello"), 0);

    SafeStrcpyN(buffer, sizeof(buffer), "asdfasdf", 8);
    EXPECT_EQ(strcmp(buffer, "asdfa"), 0);

    SafeStrcpyN(buffer, sizeof(buffer), "qwertyuiop", 3);
    EXPECT_EQ(strcmp(buffer, "qwe"), 0);
}
