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

#include <am-flags.h>
#include <stdint.h>
#include "runner.h"

using namespace ke;

enum class Scoped : uint32_t
{
  Flag0 = (1 << 0),
  Flag1 = (1 << 1),
  Flag2 = (1 << 2),
  Flag3 = (1 << 3)
};
KE_DEFINE_ENUM_OPERATORS(Scoped)

class TestFlags : public Test
{
 public:
  TestFlags()
    : Test("Flags")
  {}

  bool Run() override {
    if (!testBasic())
      return false;
    return true;
  }

 private:
  bool testBasic() {
    Flags<Scoped> flags;
    if (!check(!flags, "empty flags is false"))
      return false;
    if (flags && !check(false, "empty flags is false 2"))
      return false;

    flags += Scoped::Flag0;
    if (!check((bool)flags, "empty flags is true"))
      return false;

    flags += Scoped::Flag1;

    Flags<Scoped> other(Scoped::Flag1 | Scoped::Flag2);
    if (!check(flags != other, "different flags are not equal"))
      return false;
    if (!check(other.bits() == 0x6, "flag bits == 0x6"))
      return false;

    flags -= other;
    if (!check(flags == Scoped::Flag0, "operator -="))
      return false;

    uint32_t value = flags.bits();
    if (!check(value == 0x1, "flag bits are 0x1"))
      return false;

    flags |= other;
    if (!check(flags.bits() == 0x7, "flag bits are 0x7 after adding"))
      return false;

    flags &= other;
    if (!check(flags.bits() == 0x6, "flag bits are 0x6 after anding"))
      return false;

    return true;
  }
} sTestFlags;
