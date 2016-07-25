// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013-2015, David Anderson and AlliedModders LLC
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

#ifndef _INCLUDE_KEIMA_TPL_SHARED_PTR__H_
#define _INCLUDE_KEIMA_TPL_SHARED_PTR__H_

#include <amtl/am-autoptr.h>
#include <amtl/am-moveable.h>
#include <amtl/am-refcounting.h>

namespace ke {

// This is a helper class to assist with AutoPtrs inside lambdas. It is similar
// to a C++ shared_ptr, in that it wraps a pointer inside a reference count.
template <typename T>
class SharedPtr : public Refcounted<SharedPtr<T>>
{
 public:
  SharedPtr()
  {}
  SharedPtr(const SharedPtr& other)
   : t_(other.t__)
  {}
  SharedPtr(SharedPtr&& other)
   : t_(Move(other.t_))
  {}
  SharedPtr(T* t)
   : t_(t)
  {}
  virtual ~SharedPtr()
  {}

  T* get() const {
    return t_;
  }

 private:
  ke::AutoPtr<T> t_;
};

}

#endif /* _INCLUDE_KEIMA_TPL_SHARED_PTR__H_ */
