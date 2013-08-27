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

#include <am-thread-utils.h>
#include <am-utility.h>
#include <am-linkedlist.h>
#include "runner.h"

using namespace ke;

// Simple worker that adds up numbers.
class TestWorkerModel : public IRunnable
{
 public:
  TestWorkerModel()
   : terminate_(false),
     result_(0)
  {
  }

  void Run() {
    {
      AutoLock lock(main());
      main()->Notify();
    }

    AutoLock lock(wakeup());
    while (true) {
      while (items_.begin() != items_.end()) {
        int value = *(items_.begin());
        items_.erase(items_.begin());

        AutoUnlock unlock(wakeup());
        result_ += value;
      }

      // Only terminate if we're guaranteed the queue has no items - after
      // we've just finished processing all items in the queue, while the lock
      // is held.
      if (terminate_)
        return;

      wakeup()->Wait();
    }
  }

  ConditionVariable *main() {
    return &main_;
  }
  ConditionVariable *wakeup() {
    return &wakeup_;
  }

  void terminate() {
    AutoLock lock(&wakeup_);
    terminate_ = true;
    wakeup_.Notify();
  }

  int result() {
    return result_;
  }

  void send(int value) {
    AutoLock lock(&wakeup_);
    items_.append(value);
    wakeup_.Notify();
  }

 private:
  ConditionVariable main_;
  ConditionVariable wakeup_;
  LinkedList<int> items_;
  bool terminate_;
  int result_;
};

class TestThreading : public Test
{
 public:
  TestThreading()
   : Test("ThreadUtils")
  {
  }

  bool testWorkerModel()
  {
    TestWorkerModel test;
    ke::AutoPtr<Thread> thread(new Thread(&test, "TestWorkerModel"));
    if (!check(thread->Succeeded(), "thread launched"))
      return false;

    {
      AutoLock lock(test.main());
      if (!check(test.main()->Wait(5000) == Wait_Signaled, "thread started within five seconds"))
        return false;
    }

    int total = 0;
    for (int i = 0; i < 100000; i++) {
      test.send(i);
      total += i;
    }

    test.terminate();
    thread->Join();

    if (!check(test.result() == total, "thread returned correct result"))
      return false;

    return true;
  }

  bool Run() KE_OVERRIDE
  {
    if (!testWorkerModel())
      return false;
    return true;
  }
} sTestThreading;

