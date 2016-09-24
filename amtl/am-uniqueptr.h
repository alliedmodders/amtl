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

#ifndef _include_amtl_uniqueptr_h_
#define _include_amtl_uniqueptr_h_

#include <assert.h>
#include <amtl/am-cxx.h>
#include <amtl/am-moveable.h>
#include <amtl/am-raii.h>
#include <amtl/am-type-traits.h>

namespace ke {

// Wrapper that automatically deletes its contents. The pointer can be taken
// to avoid destruction.
template <typename T>
class UniquePtr
{
 public:
  UniquePtr()
   : t_(nullptr)
  {
  }
  explicit UniquePtr(T *t)
   : t_(t)
  {
  }
  UniquePtr(UniquePtr &&other)
  {
    t_ = other.t_;
    other.t_ = nullptr;
  }
  ~UniquePtr() {
    delete t_;
  }
  T *get() const {
    return t_;
  }
  T *take() {
    return ReturnAndVoid(t_);
  }
  void assign(T* ptr) {
    delete t_;
    t_ = ptr;
  }
  T *operator *() const {
    return t_;
  }
  T *operator ->() const {
    return t_;
  }
  T *operator =(UniquePtr &&other) {
    assign(other.take());
    return t_;
  }
  UniquePtr& operator =(decltype(nullptr)) {
    assign(nullptr);
    return *this;
  }
  explicit operator bool() const {
    return t_ != nullptr;
  }

 private:
  UniquePtr(const UniquePtr &other) = delete;
  UniquePtr &operator =(const UniquePtr &other) = delete;

 private:
  T *t_;
};

// Wrapper that automatically deletes its contents. The pointer can be taken
// to avoid destruction.
template <typename T>
class UniquePtr<T[]>
{
 public:
  UniquePtr()
   : t_(nullptr)
  {
  }
  UniquePtr(UniquePtr&& other)
    : t_(other.t_)
  {
    other.t_ = nullptr;
  }
  explicit UniquePtr(T *t)
   : t_(t)
  {
  }
  ~UniquePtr() {
    delete [] t_;
  }
  T *get() const {
    return t_;
  }
  T *take() {
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

  UniquePtr& operator =(decltype(nullptr)) {
    assign(nullptr);
    return *this;
  }
  UniquePtr& operator =(UniquePtr&& other) {
    assign(other.take());
    return *this;
  }

 private:
  UniquePtr(const UniquePtr& other) = delete;
  UniquePtr& operator =(const UniquePtr& other) = delete;

 private:
  T *t_;
};

namespace impl {

// From N3656.
template <typename T>
struct UniquePtrMatcher {
  typedef UniquePtr<T> SingleObject;
};

template <typename T>
struct UniquePtrMatcher<T[]> {
  typedef UniquePtr<T[]> UnknownBound;
};

template <typename T, size_t N>
struct UniquePtrMatcher<T[N]> {
  typedef void KnownBound;
};

} // namespace impl

// C++14 make_unique port.
template <typename T, typename ... Args>
typename impl::UniquePtrMatcher<T>::SingleObject
MakeUnique(Args&&... args)
{
  return UniquePtr<T>(new T(Forward<Args>(args)...));
}

template <typename T>
typename impl::UniquePtrMatcher<T>::UnknownBound
MakeUnique(size_t count)
{
  typedef typename remove_extent<T>::type BaseType;
  return UniquePtr<T>(new BaseType[count]());
}

// Forbidden to use T[N] or T[](args).
template <typename T, typename ... Args>
typename impl::UniquePtrMatcher<T>::KnownBound
MakeUnique(Args&&... args) = delete;

} // namespace ke

#endif // _include_amtl_uniqueptr_h_
