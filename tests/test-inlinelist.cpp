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

#include <amtl/am-inlinelist.h>
#include <amtl/am-utility.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

class IntThing : public InlineListNode<IntThing>
{
  public:
    IntThing(int value)
     : value_(value)
    {}
    ~IntThing() {}
    int value() const {
        return value_;
    }

  private:
    int value_;
};

TEST(InlineList, Basic) {
    IntThing thing1(1);
    IntThing thing2(2);
    IntThing thing3(3);
    IntThing thing4(4);
    IntThing thing5(5);

    // Do these test twice. InlineList does not take ownership of pointers,
    // so we should be guaranteed we can keep moving them in between lists.
    for (size_t i = 0; i <= 1; i++) {
        InlineList<IntThing> list;

        InlineList<IntThing>::iterator iter = list.begin();
        ASSERT_EQ(iter, list.end());

        list.append(&thing1);
        list.append(&thing2);
        list.append(&thing3);
        list.append(&thing4);
        list.append(&thing5);

        iter = list.begin();
        for (int n = 1; n <= 5; n++) {
            EXPECT_EQ(iter->value(), n);
            iter++;
        }
        EXPECT_EQ(iter, list.end());

        list.remove(&thing1);
        iter = list.begin();
        EXPECT_EQ(iter->value(), 2);

        list.remove(&thing5);
        iter = list.begin();
        iter++;
        iter++;
        EXPECT_EQ(iter->value(), 4);
        iter++;
        EXPECT_EQ(iter, list.end());

        list.remove(&thing3);
        iter = list.begin();
        EXPECT_EQ(iter->value(), 2);
        iter++;
        EXPECT_EQ(iter->value(), 4);
        iter++;
        EXPECT_EQ(iter, list.end());

        iter = list.begin();
        while (iter != list.end())
            iter = list.erase(iter);

        EXPECT_EQ(list.begin(), list.end());
    }
}
