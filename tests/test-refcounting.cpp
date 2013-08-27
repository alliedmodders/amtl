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

#include <am-utility.h>
#include <am-refcounting.h>
#include "runner.h"

using namespace ke;

static size_t sDtors = 0;

class Counted : public Refcounted<Counted>
{
 public:
  ~Counted() {
    sDtors++;
  }
};

static inline PassRef<Counted>
PassThrough(const Ref<Counted> &obj)
{
  return obj;
}

class TestRefcounting : public Test
{
 public:
  TestRefcounting()
   : Test("Refcounting")
  {
  }

  bool Run() KE_OVERRIDE
  {
    {
      Ref<Counted> obj(Newborn<Counted>(new Counted()));
    }
    if (!check(sDtors == 1, "Ref/Newborn counted properly"))
      return false;

    {
      Ref<Counted> obj(Newborn<Counted>(new Counted()));
      Ref<Counted> obj2 = PassThrough(obj);
      Ref<Counted> obj3 = PassThrough(obj);
      if (!check(sDtors == 1, "destructor not called early"))
        return false;
      Ref<Counted> obj4 = PassThrough(PassThrough(PassThrough(obj)));
      if (!check(sDtors == 1, "destructor not called early"))
        return false;
    }
    if (!check(sDtors == 2, "PassRef/Ref counted properly"))
      return false;

    return true;
  }
} sTestRefcounting;

