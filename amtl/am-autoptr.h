// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013-2014, David Anderson and AlliedModders LLC
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

#ifndef _include_amtl_autoptr_h_
#define _include_amtl_autoptr_h_

#include <assert.h>
#include <amtl/am-cxx.h>
#include <amtl/am-moveable.h>
#include <amtl/am-raii.h>
#include <amtl/am-type-traits.h>
#include <amtl/am-uniqueptr.h>

namespace ke {

// Wrapper that automatically deletes its contents. The pointer can be taken
// to avoid destruction.
template <typename T>
class AutoPtr
{
 public:
  AutoPtr()
   : t_(nullptr)
  {
  }
  explicit AutoPtr(T *t)
   : t_(t)
  {
  }
  AutoPtr(UniquePtr<T>&& other)
   : t_(other.take())
  {
  }
  AutoPtr(AutoPtr &&other)
   : t_(other.take())
  {
  }
  ~AutoPtr() {
    delete t_;
  }
  T *get() const {
    return t_;
  }
  T *take() {
    return ReturnAndVoid(t_);
  }
  T *operator *() const {
    return t_;
  }
  T *operator ->() const {
    return t_;
  }
  operator T* () const {
    return t_;
  }
  void assign(T* t) {
    delete t_;
    t_ = t;
  }
  AutoPtr& operator =(decltype(nullptr)) {
    assign(nullptr);
    return *this;
  }
  AutoPtr& operator =(T* t) {
    assign(t);
    return *this;
  }
  AutoPtr& operator =(AutoPtr &&other) {
    assign(other.take());
    return *this;
  }
  AutoPtr& operator =(UniquePtr<T> &&other) {
    assign(other.take());
    return *this;
  }
  explicit operator bool() const {
    return !!t_;
  }

 private:
  AutoPtr(const AutoPtr &other) = delete;
  AutoPtr &operator =(const AutoPtr &other) = delete;

 private:
  T *t_;
};

// Wrapper that automatically deletes its contents. The pointer can be taken
// to avoid destruction.
template <typename T>
class AutoPtr<T[]>
{
 public:
  AutoPtr()
   : t_(nullptr)
  {
  }
  AutoPtr(AutoPtr&& other)
   : t_(other.take())
  {
  }
  AutoPtr(UniquePtr<T[]>&& other)
   : t_(other.take())
  {
  }
  explicit AutoPtr(T *t)
   : t_(t)
  {
  }
  ~AutoPtr() {
    delete [] t_;
  }
  T *get() const {
    return t_;
  }
  T *take() {
    return ReturnAndVoid(t_);
  }
  T *forget() {
    return ReturnAndVoid(t_);
  }
  explicit operator bool() const {
    return t_ != nullptr;
  }

  void assign(T* ptr) {
    delete[] t_;
    t_ = ptr;
  }

  T& operator[](size_t index) {
    return t_[index];
  }

  AutoPtr& operator =(decltype(nullptr)) {
    assign(nullptr);
    return *this;
  }
  AutoPtr& operator =(AutoPtr&& other) {
    assign(other.take());
    return *this;
  }
  AutoPtr& operator =(UniquePtr<T[]>&& other) {
    assign(other.take());
    return *this;
  }

 private:
  AutoPtr(const AutoPtr& other) = delete;
  AutoPtr& operator =(const AutoPtr& other) = delete;

 private:
  T *t_;
};

} // namespace ke

#endif // _include_amtl_autoptr_h_
