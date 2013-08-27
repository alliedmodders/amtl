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

#ifndef _include_amtl_runner_h_
#define _include_amtl_runner_h_

#include <stdio.h>
#include <stdarg.h>

namespace ke {

class Test
{
 public:
  Test(const char *name)
   : name_(name),
     next_(head)
  {
    head = this;
  }

  virtual bool Run() = 0;

  const char *name() const {
    return name_;
  }
  Test *next() const {
    return next_;
  }

  static inline Test *first() {
    return head;
  }

 private:
  static Test *head;

 private:
  const char *name_;
  Test *next_;
};

static inline bool
check(bool condition, const char *fmt, ...)
{
  FILE *fp = condition ? stdout : stderr;
  if (condition)
    fprintf(fp, " -- Ok: ");
  else
    fprintf(fp, " -- Failure: ");
  va_list ap;
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
  fprintf(fp, "\n");
  return condition;
}

class FallibleMalloc
{
 public:
  FallibleMalloc()
   : shouldOutOfMemory_(false),
     ooms_(0),
     overflows_(0)
  {
  }

  void *malloc(size_t amount) {
    if (shouldOutOfMemory_) {
      reportOutOfMemory();
      return NULL;
    }
    return ::malloc(amount);
  }
  void free(void *p) {
    return ::free(p);
  }
  void reportOutOfMemory() {
    ooms_++;
  }
  void reportAllocationOverflow() {
    overflows_++;
  }

  void setOutOfMemory(bool oom) {
    shouldOutOfMemory_ = oom;
  }
  size_t ooms() const {
    return ooms_;
  }
  size_t overflows() const {
    return overflows_;
  }

 private:
  bool shouldOutOfMemory_;
  size_t ooms_;
  size_t overflows_;
};

}

#endif // _include_amtl_runner_h_

