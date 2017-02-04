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

template <typename T>
struct DefaultDeleter
{
  void operator()(T *ptr) const {
    delete ptr;
  }
};

template <typename T>
struct DefaultDeleter<T[]>
{
  void operator()(T *ptr) const {
    delete[] ptr;
  }
};

// Wrapper that automatically deletes its contents. The pointer can be taken
// to avoid destruction.
template <typename T, typename Deleter = DefaultDeleter<T>>
class AutoPtr
{
 public:
  AutoPtr()
   : t_(nullptr),
    deleter_()
  {
  }
  explicit AutoPtr(T *t)
   : t_(t),
    deleter_()
  {
  }
  AutoPtr(UniquePtr<T>&& other)
   : t_(other.take()),
    deleter_()
  {
  }
  AutoPtr(AutoPtr &&other)
   : t_(other.take()),
    deleter_(Move(other.deleter_))
  {
  }
  ~AutoPtr() {
    deleter_(t_);
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
    deleter_(t_);
    t_ = t;
  }
  Deleter &getDeleter() const {
    return deleter_;
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
    deleter_ = Move(other.deleter_);
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
  Deleter deleter_;
};

// Wrapper that automatically deletes its contents. The pointer can be taken
// to avoid destruction.
template <typename T, typename Deleter>
class AutoPtr<T[], Deleter>
{
 public:
  AutoPtr()
   : t_(nullptr),
    deleter_()
  {
  }
  AutoPtr(AutoPtr&& other)
   : t_(other.take()),
    deleter_(Move(other.deleter_))
  {
  }
  AutoPtr(UniquePtr<T[]>&& other)
   : t_(other.take()),
    deleter_()
  {
  }
  explicit AutoPtr(T *t)
   : t_(t),
    deleter_()
  {
  }
  ~AutoPtr() {
    deleter_(t_);
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
    deleter_(t_);
    t_ = ptr;
  }
  Deleter &getDeleter() const {
    return deleter_;
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
    deleter_ = Move(other.deleter_);
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
  Deleter deleter_;
};

} // namespace ke

#endif // _include_amtl_autoptr_h_
