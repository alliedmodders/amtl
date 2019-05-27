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

#include <amtl/am-refcounting-threadsafe.h>
#include <amtl/am-refcounting.h>
#include <amtl/am-utility.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include "runner.h"

using namespace ke;

static int sDtors = 0;

class Counted : public Refcounted<Counted>
{
  public:
    ~Counted() {
        sDtors++;
    }
};

class SubCounted : public Counted
{
};

void
TypeChecks_DoNotCall()
{
    RefPtr<Counted> counted;
    if (counted)
        abort();
}

static inline RefPtr<Counted>
PassThrough(const RefPtr<Counted>& obj)
{
    return obj;
}

TEST(RefPtr, Basic) {
    { RefPtr<Counted> obj(new Counted()); }
    EXPECT_EQ(sDtors, 1);
    { RefPtr<Counted> obj(new Counted()); }
    EXPECT_EQ(sDtors, 2);
    {
        Counted* counted = new Counted();
        counted->AddRef();
        RefPtr<Counted> obj(AdoptRef(counted));
    }
    EXPECT_EQ(sDtors, 3);

    // Check that subclass assignment works.
    {
        RefPtr<Counted> obj(new SubCounted());
        RefPtr<Counted> obj2(PassThrough(new SubCounted()));
    }
    EXPECT_EQ(sDtors, 5);

    sDtors = 0;

    {
        RefPtr<Counted> obj(new Counted());
        RefPtr<Counted> obj2 = PassThrough(obj);
        RefPtr<Counted> obj3 = PassThrough(obj);
        EXPECT_EQ(sDtors, 0);
        RefPtr<Counted> obj4 = PassThrough(PassThrough(PassThrough(obj)));
        EXPECT_EQ(sDtors, 0);
    }
    EXPECT_EQ(sDtors, 1);

    sDtors = 0;
    { AtomicRef<Counted> obj(new Counted()); }
    EXPECT_EQ(sDtors, 1);

    sDtors = 0;
    {
        AtomicRef<Counted> obj;
        obj = new Counted();
        obj = new Counted();
        obj = nullptr;
    }
    EXPECT_EQ(sDtors, 2);

    sDtors = 0;
    {
        RefPtr<Counted> obj(new Counted());
        AlreadyRefed<Counted> xfer = obj.take();
        EXPECT_FALSE(obj);
        EXPECT_TRUE(!!xfer);
    }
    EXPECT_EQ(sDtors, 1);
}
