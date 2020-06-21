// vim: set sts=4 ts=8 sw=4 tw=99 et:
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
    std::unique_ptr<char[]> ptr = Sprintf("A: %d B: %s", a, value);
    EXPECT_EQ(strcmp(ptr.get(), expect), 0);

    auto str = StringPrintf("A: %d B: %s", a, value);
    EXPECT_EQ(str, expect);
}

TEST(String, Split) {
    auto out = Split("     ", " ");
    EXPECT_EQ(out.size(), (size_t)6);
    for (size_t i = 0; i < out.size(); i++) {
        EXPECT_EQ(out[i].size(), (size_t)0);
    }

    out = Split("egg", " ");
    EXPECT_EQ(out.size(), (size_t)1);
    EXPECT_EQ(out[0].compare("egg"), 0);

    out = Split("", "egg");
    EXPECT_EQ(out.size(), (size_t)0);

    out = Split("xaba", "a");
    EXPECT_EQ(out.size(), (size_t)3);
    EXPECT_EQ(out[0].compare("x"), 0);
    EXPECT_EQ(out[1].compare("b"), 0);
    EXPECT_EQ(out[2].compare(""), 0);

    out = Split("egg ham", " ");
    EXPECT_EQ(out.size(), (size_t)2);
    EXPECT_EQ(out[0].compare("egg"), 0);
    EXPECT_EQ(out[1].compare("ham"), 0);
}

TEST(String, Join) {
    std::vector<std::string> in;

    auto result = Join(in, "x");
    EXPECT_EQ(result, "");

    in.emplace_back("abc");
    result = Join(in, "x");
    EXPECT_EQ(result, "abc");

    in.emplace_back("xyz");
    result = Join(in, "T");
    EXPECT_EQ(result, "abcTxyz");

    in.emplace_back("def");
    result = Join(in, "");
    EXPECT_EQ(result, "abcxyzdef");

    in = {"a", "b", "", "d" };
    result = Join(in, ",");
    EXPECT_EQ(result, "a,b,,d");

    in = {"a", "b", "c", "" };
    result = Join(in, ",");
    EXPECT_EQ(result, "a,b,c,");
}

TEST(String, Case) {
    const char* str = "samPle1.com";
    EXPECT_EQ(Uppercase(str), "SAMPLE1.COM");
    EXPECT_EQ(Lowercase(str), "sample1.com");

    EXPECT_EQ(Lowercase(""), "");
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

TEST(String, StartsWith) {
    std::string str("blah");

    EXPECT_TRUE(StartsWith(str, "b"));
    EXPECT_TRUE(StartsWith(str, "blah"));
    EXPECT_FALSE(StartsWith(str, "a"));
    EXPECT_FALSE(StartsWith(str, "blah2"));
}

TEST(String, EndsWith) {
    std::string str("blah");

    EXPECT_TRUE(EndsWith(str, "h"));
    EXPECT_TRUE(EndsWith(str, "blah"));
    EXPECT_FALSE(EndsWith(str, "a"));
    EXPECT_FALSE(EndsWith(str, "blah2"));
}

TEST(String, SafeStrcat) {
    char buffer[6] = "hello";

    buffer[1] = '\0';
    EXPECT_EQ(SafeStrcat(buffer, 0, "blah"), (size_t)1);
    EXPECT_EQ(SafeStrcat(buffer, 1, "blah"), (size_t)1);
    EXPECT_EQ(SafeStrcat(buffer, 2, "blah"), (size_t)1);
    EXPECT_EQ(buffer[1], '\0');
    EXPECT_EQ(buffer[2], 'l');

    EXPECT_EQ(SafeStrcat(buffer, sizeof(buffer), "b"), (size_t)2);
    EXPECT_EQ(strcmp(buffer, "hb"), 0);

    EXPECT_EQ(SafeStrcat(buffer, sizeof(buffer), "hamocrab"), (size_t)5);
    EXPECT_EQ(strcmp(buffer, "hbham"), 0);
}
