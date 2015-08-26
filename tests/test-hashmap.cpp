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

#include <am-hashmap.h>
#include <am-utility.h>
#include <am-string.h>
#include "runner.h"

using namespace ke;

struct StringPolicy
{
  static inline uint32_t hash(const char *key) {
    return FastHashCharSequence(key, strlen(key));
  }
  static inline bool matches(const char *find, const AString &key) {
    return key.compare(find) == 0;
  }
};

class TestHashMap : public Test
{
 public:
  TestHashMap()
   : Test("HashMap")
  {
  }

  bool testBasics() {
    typedef HashMap<AString, int, StringPolicy> Map;
    Map map;

    if (!check(map.init(), "initialized"))
      return false;

    Map::Result r = map.find("cat");
    if (!check(!r.found(), "cat not found"))
      return false;

    Map::Insert i = map.findForAdd("cat");
    if (!check(!i.found(), "cat not found for add"))
      return false;
    if (!check(map.add(i, AString("cat"), 5), "cat added"))
      return false;
    if (!check(r->value == 5, "cat is 5"))
      return false;

    Map::iterator iter = map.iter();
    if (!check(iter->key.compare("cat") == 0, "iterator got key cat"))
      return false;
    if (!check(iter->value == 5, "iterator got value 5"))
      return false;
    iter.next();
    if (!check(iter.empty(), "iterator should be finished"))
      return false;

    i = map.findForAdd("cat");
    if (!check(i.found(), "cat found through insert"))
      return false;

    r = map.find("dog");
    if (!check(!r.found(), "dog not found"))
      return false;

    r = map.find("cat");
    if (!check(r.found(), "cat found"))
      return false;
    if (!check(r->value == 5, "cat is 5"))
      return false;
    map.remove(r);

    r = map.find("cat");
    if (!check(!r.found(), "cat ran away"))
      return false;

    return true;
  }

  bool Run() override
  {
    if (!testBasics())
      return false;
    return true;
  }
} sTestHashMap;

