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

#include <am-threadlocal.h>
#include "runner.h"

using namespace ke;

static ThreadLocal<int> sThreadVar;
static ThreadLocal<void *> sThreadVarPointer;

class VarThread : public IRunnable
{
 public:
  VarThread()
   : succeeded_(false)
  {
  }

  void Run() override {
    if (!check(sThreadVar == 0, "value starts as 0 on new thread"))
      return;

    sThreadVar = 20;
    if (!check(sThreadVar == 20, "value is set correctly"))
      return;

    succeeded_ = true;
  }

  bool succeeded() const {
    return succeeded_;
  }

 private:
  bool succeeded_;
};

class TestThreadLocalThreaded : public Test
{
 public:
  TestThreadLocalThreaded()
   : Test("ThreadLocalThreaded")
  {
  }

  bool Run() override
  {
    sThreadVar = 10;

    VarThread run;
    ke::AutoPtr<Thread> thread(new Thread(&run, "TestThreadLocal"));
    if (!check(thread->Succeeded(), "thread launched"))
      return false;
    thread->Join();

    if (!check(run.succeeded(), "thread completed correctly"))
      return false;
    if (!check(sThreadVar == 10, "value did not change on main thread"))
      return false;

    // Check that pointers are allowed in T.
    sThreadVarPointer = &run;
    if (!check(sThreadVarPointer == &run, "pointer value sets properly"))
      return false;

    return true;
  }
} sTestThreadLocalThreaded;
