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

#include <amtl/am-hashmap.h>
#include <amtl/am-string.h>
#include <amtl/am-utility.h>
#include <gtest/gtest.h>
#include "runner.h"

using namespace ke;

struct StringPolicy {
    static inline uint32_t hash(const char* key) {
        return FastHashCharSequence(key, strlen(key));
    }
    static inline bool matches(const char* find, const std::string& key) {
        return key.compare(find) == 0;
    }
};

TEST(HashMap, Basic) {
    typedef HashMap<std::string, int, StringPolicy> Map;
    Map map;

    ASSERT_TRUE(map.init());

    Map::Result r = map.find("cat");
    ASSERT_FALSE(r.found());

    Map::Insert i = map.findForAdd("cat");
    ASSERT_FALSE(i.found());
    ASSERT_TRUE(map.add(i, std::string("cat"), 5));
    EXPECT_EQ(r->value, 5);

    Map::iterator iter = map.iter();
    EXPECT_EQ(iter->key.compare("cat"), 0);
    EXPECT_EQ(iter->value, 5);
    iter.next();
    EXPECT_TRUE(iter.empty());

    i = map.findForAdd("cat");
    EXPECT_TRUE(i.found());

    r = map.find("dog");
    EXPECT_FALSE(r.found());

    r = map.find("cat");
    ASSERT_TRUE(r.found());
    EXPECT_EQ(r->value, 5);
    map.remove(r);

    r = map.find("cat");
    EXPECT_FALSE(r.found());
}

TEST(HashMap, Bug6527) {
    typedef HashMap<std::string, int, StringPolicy> Map;
    Map map;

    ASSERT_TRUE(map.init(16));

    {
        char key[] = "bb";
        Map::Insert p = map.findForAdd(key);
        ASSERT_FALSE(p.found());
        ASSERT_TRUE(map.add(p, key));
        p->value = 0xabab;
    }

    {
        char key[] = "dddd";
        Map::Insert p = map.findForAdd(key);
        ASSERT_FALSE(p.found());
        ASSERT_TRUE(map.add(p, key));
        p->value = 0xacac;
    }

    {
        char key[] = "bb";
        map.removeIfExists(key);
    }

    {
        char key[] = "dddd";
        Map::Insert p = map.findForAdd(key);
        if (!p.found())
            map.add(p, key);
        p->value = 0xadad;
    }

    bool found = false;
    for (Map::iterator iter = map.iter(); !iter.empty(); iter.next()) {
        if (iter->key.compare("dddd") == 0) {
            // Should only occur once.
            EXPECT_FALSE(found);
            found = true;
        }
    }
}

struct Int32Policy {
    static inline uint32_t hash(uint32_t key) {
        return HashInt32(key);
    }
    static inline bool matches(uint32_t find, uint32_t key) {
        return find == key;
    }
};

struct Int64Policy {
    static inline uint32_t hash(uint64_t key) {
        return uint32_t(HashInt64(key));
    }
    static inline bool matches(uint64_t find, uint64_t key) {
        return find == key;
    }
};

// Integer keys, spread over all bits: HashInt32's and HashInt64's arithmetic overflows for them,
// which must wrap around rather than be undefined (an optimiser may compute a hash differently where
// the key is a constant, so that a lookup misses a key that was added).
TEST(HashMap, IntegerKeys) {
    HashMap<uint32_t, uint32_t, Int32Policy> map32;
    HashMap<uint64_t, uint32_t, Int64Policy> map64;
    ASSERT_TRUE(map32.init());
    ASSERT_TRUE(map64.init());

    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t key32 = i * 2654435761u;
        uint64_t key64 = (uint64_t(key32) << 32) | i;

        auto insert32 = map32.findForAdd(key32);
        ASSERT_FALSE(insert32.found());
        ASSERT_TRUE(map32.add(insert32, key32, i));

        auto insert64 = map64.findForAdd(key64);
        ASSERT_FALSE(insert64.found());
        ASSERT_TRUE(map64.add(insert64, key64, i));
    }
    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t key32 = i * 2654435761u;
        uint64_t key64 = (uint64_t(key32) << 32) | i;

        auto result32 = map32.find(key32);
        ASSERT_TRUE(result32.found());
        EXPECT_EQ(result32->value, i);

        auto result64 = map64.find(key64);
        ASSERT_TRUE(result64.found());
        EXPECT_EQ(result64->value, i);
    }
}
