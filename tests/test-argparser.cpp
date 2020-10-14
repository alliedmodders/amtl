// vim: set sts=8 ts=4 sw=4 tw=99 et:
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

#include <amtl/experimental/am-argparser.h>
#include <gtest/gtest.h>
#include <limits.h>
#include "runner.h"

using namespace ke;
using namespace ke::args;

TEST(ArgParser, Basic) {
    Parser parser("help");

    EXPECT_TRUE(parser.parsev(nullptr));
    EXPECT_FALSE(parser.parsev("asdf", nullptr));
    EXPECT_FALSE(parser.parsev("--asdf", nullptr));
}

TEST(ArgParser, ToggleArg) {
    Parser parser("help");

    ToggleOption x(parser, "x", nullptr, Nothing(), "help");
    ToggleOption y(parser, "y", nullptr, Some(true), "help");
    ToggleOption z(parser, "z", nullptr, Some(false), "help");

    parser.reset();
    EXPECT_TRUE(parser.parsev(nullptr));
    EXPECT_FALSE(x.value());

    parser.reset();
    EXPECT_TRUE(parser.parsev("-x", nullptr));
    EXPECT_TRUE(x.value());

    parser.reset();
    EXPECT_TRUE(parser.parsev("-y", nullptr));
    EXPECT_FALSE(y.value());

    parser.reset();
    EXPECT_TRUE(parser.parsev("-z", nullptr));
    EXPECT_TRUE(z.value());

    parser.reset();
    EXPECT_FALSE(parser.parsev("-z=false", nullptr));
}

TEST(ArgParser, StringArg) {
    Parser parser("help");

    StringOption s(parser, "s", "string", Nothing(), "help");
    StringOption t(parser, "t", "ttt", Some(std::string("whatever")), "help");
    StringOption mode(parser, "mode", "help");

    parser.reset();
    EXPECT_FALSE(parser.parsev(nullptr));

    parser.reset();
    EXPECT_TRUE(parser.parsev("crab", nullptr));
    EXPECT_FALSE(s.hasValue());
    EXPECT_EQ(t.value().compare("whatever"), 0);
    EXPECT_EQ(mode.value().compare("crab"), 0);

    parser.reset();
    EXPECT_TRUE(parser.parsev("-s", "yam", "egg", nullptr));
    EXPECT_EQ(s.value().compare("yam"), 0);
    EXPECT_EQ(mode.value().compare("egg"), 0);
}

TEST(ArgParser, IntArg) {
    Parser parser("help");

    IntOption val1(parser, nullptr, "val", Nothing(), "help");

    parser.reset();
    EXPECT_TRUE(parser.parsev("--val", "308", nullptr));
    EXPECT_EQ(val1.value(), 308);

    parser.reset();
    EXPECT_FALSE(parser.parsev("--val", "30x", nullptr));
}

TEST(ArgParser, RepeatArg) {
    Parser parser("help");

    RepeatOption<std::string> inc(parser, "-i", "--include-path", "Include path.");

    ASSERT_TRUE(parser.parsev("-i", "blah", "-i", "crab", "--include-path=yam", nullptr));

    std::vector<std::string> values = std::move(inc.values());
    ASSERT_EQ(values.size(), (size_t)3);
    EXPECT_EQ(values[0].compare("blah"), 0);
    EXPECT_EQ(values[1].compare("crab"), 0);
    EXPECT_EQ(values[2].compare("yam"), 0);
}

TEST(ArgParser, StopArg1) {
    Parser parser("help");

    StopOption show_version(parser, "-v", "--version", Some(false), "Show the version and exit.");
    StringOption required(parser, "something_required", "This is a required positional argument.");

    EXPECT_FALSE(parser.parsev(nullptr));
    EXPECT_TRUE(parser.parsev("-v", nullptr));
    EXPECT_TRUE(show_version.value());
}

TEST(ArgParser, StopArg2) {
    Parser parser("help");

    StopOption show_version(parser, "-v", "--version", Some(false), "Show the version and exit.");
    StringOption filename(parser, "file", "SMX file to execute.");

    EXPECT_TRUE(parser.parsev("-v", "-w", nullptr));
    EXPECT_TRUE(show_version.value());
}

TEST(ArgParser, NoSeparator) {
    Parser parser("help");
    parser.enable_inline_values();

    IntOption opt(parser, "-O", nullptr, {}, "Optimization level");

    ASSERT_TRUE(parser.parsev("-O2", nullptr));
    EXPECT_EQ(opt.value(), 2);
}

TEST(ArgParser, ColonSeparator) {
    Parser parser("help");
    parser.enable_inline_values();

    IntOption opt(parser, "-O", nullptr, {}, "Optimization level");

    ASSERT_TRUE(parser.parsev("-O:2", nullptr));
    EXPECT_EQ(opt.value(), 2);
}

TEST(ArgParser, Slashes) {
    Parser parser("help");
    parser.enable_inline_values();
    parser.allow_slashes();

    IntOption opt(parser, "-O", nullptr, {}, "Optimization level");

    ASSERT_TRUE(parser.parsev("/O:2", nullptr));
    EXPECT_EQ(opt.value(), 2);
}

TEST(ArgParser, CollectExtra) {
    Parser parser("help");
    parser.collect_extra_args();

    ASSERT_TRUE(parser.parsev("a", "b", "c", nullptr));
    ASSERT_EQ(parser.extra_args().size(), (unsigned)3);
    EXPECT_EQ(parser.extra_args()[0].compare("a"), 0);
    EXPECT_EQ(parser.extra_args()[1].compare("b"), 0);
    EXPECT_EQ(parser.extra_args()[2].compare("c"), 0);
}

TEST(ArgParser, OptionWithNo) {
    Parser parser("help");

    EnableOption stuff(parser, nullptr, "--stuff", false, "Stuff or whatever");
    EnableOption other(parser, nullptr, "--other", true, "Other or whatever");

    ASSERT_TRUE(parser.parsev("--stuff", "--other", nullptr));
    ASSERT_TRUE(stuff.value());
    ASSERT_TRUE(other.value());

    parser.reset();

    ASSERT_TRUE(parser.parsev("--no-stuff", "--no-other", nullptr));
    ASSERT_FALSE(stuff.value());
    ASSERT_FALSE(other.value());
}
