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

#ifndef _INCLUDE_KEIMA_TPL_CPP_VECTOR_H_
#define _INCLUDE_KEIMA_TPL_CPP_VECTOR_H_

#include <new>
#include <stdlib.h>
#include <amtl/am-allocator-policies.h>
#include <amtl/am-bits.h>
#include <amtl/am-moveable.h>

namespace ke {

template <typename T, typename AllocPolicy = SystemAllocatorPolicy>
class Vector : private AllocPolicy
{
 public:
  explicit Vector(AllocPolicy = AllocPolicy())
   : data_(nullptr),
     nitems_(0),
     maxsize_(0)
  {
  }

  Vector(Vector&& other)
   : AllocPolicy(Move(other))
  {
    data_ = other.data_;
    nitems_ = other.nitems_;
    maxsize_ = other.maxsize_;
    other.reset();
  }

  ~Vector() {
    zap();
  }

  template <typename U>
  bool append(U&& item) {
    if (!growIfNeeded(1))
      return false;
    new (&data_[nitems_]) T(ke::Forward<U>(item));
    nitems_++;
    return true;
  }
  template <typename U>
  void infallibleAppend(U&& item) {
    assert(growIfNeeded(1));
    new (&data_[nitems_]) T(ke::Forward<U>(item));
    nitems_++;
  }

  // Shift all elements including |at| up by one, and insert |item| at the
  // given position. If |at| is one greater than the last usable index,
  // i.e. |at == length()|, then this is the same as append(). No other
  // invalid indexes are allowed.
  //
  // This is a linear-time operation.
  template <typename U>
  bool insert(size_t at, U&& item) {
    if (at == length())
      return append(ke::Forward<U>(item));
    if (!moveUp(at))
      return false;
    new (&data_[at]) T(ke::Forward<U>(item));
    return true;
  }

  AllocPolicy& allocPolicy() {
    return *this;
  }
  const AllocPolicy& allocPolicy() const {
    return *this;
  }

  // Shift all elements at the given position down, removing the given
  // element. This is a linear-time operation.
  void remove(size_t at) {
    for (size_t i = at; i < length() - 1; i++)
      data_[i] = ke::Move(data_[i + 1]);
    pop();
  }

  T popCopy() {
    T t = at(length() - 1);
    pop();
    return t;
  }
  void pop() {
    assert(nitems_);
    data_[nitems_ - 1].~T();
    nitems_--;
  }
  bool empty() const {
    return length() == 0;
  }
  size_t length() const {
    return nitems_;
  }
  T& at(size_t i) {
    assert(i < length());
    return data_[i];
  }
  const T& at(size_t i) const {
    assert(i < length());
    return data_[i];
  }
  T& operator [](size_t i) {
    return at(i);
  }
  const T& operator [](size_t i) const {
    return at(i);
  }
  void clear() {
    destruct_live();
    nitems_ = 0;
  }
  const T& back() const {
    return at(length() - 1);
  }
  T& back() {
    return at(length() - 1);
  }

  T* buffer() {
    return data_;
  }
  const T* buffer() const {
    return data_;
  }

  bool resize(size_t newLength) {
    if (newLength < length()) {
      while (newLength < length())
        pop();
    } else if (newLength > length()) {
      if (!ensure(newLength))
        return false;
      size_t count = newLength - length();
      for (size_t i = 0; i < count; i++)
        infallibleAppend(T());
    }
    return true;
  }
  bool ensure(size_t desired) {
    if (desired <= length())
      return true;

    return growIfNeeded(desired - length());
  }

  template <typename U>
  bool extend(U&& other) {
    if (length() == 0) {
      *this = Move(other);
    } else {
      for (size_t i = 0; i < other.length(); i++) {
        if (!append(Move(other[i])))
          return false;
      }
    }
    return true;
  }

  Vector& operator =(Vector&& other) {
    zap();
    data_ = other.data_;
    nitems_ = other.nitems_;
    maxsize_ = other.maxsize_;
    other.reset();
    return *this;
  }

 public:
  // Support for C++11 iteration. Note that the vector length must not be
  // mutated inside such an iterator.
  T* begin() {
    return data_;
  }
  const T* begin() const {
    return data_;
  }
  T* end() {
    return data_ + nitems_;
  }
  const T* end() const {
    return data_ + nitems_;
  }

 private:
  // These are disallowed because they basically violate the failure handling
  // model for AllocPolicies and are also likely to have abysmal performance.
  Vector(const Vector& other) = delete;
  Vector& operator =(const Vector& other) = delete;

 private:
  void destruct_live() {
    for (size_t i = 0; i < nitems_; i++)
      data_[i].~T();
  }
  void zap() {
    destruct_live();
    this->am_free(data_);
  }
  void reset() {
    data_ = nullptr;
    nitems_ = 0;
    maxsize_ = 0;
  }

  bool moveUp(size_t at) {
    // Note: we don't use append() here. Passing an element as a Moveable into
    // insert() or append() can break, since the underlying storage could be
    // reallocated, invalidating the Moveable reference. Instead, we inline
    // the logic to append() to ensure growIfNeeded occurs before any
    // references are taken.
    if (!growIfNeeded(1))
      return false;
    new (&data_[nitems_]) T(ke::Move(data_[nitems_ - 1]));
    nitems_++;
    for (size_t i = nitems_ - 2; i > at; i--)
      data_[i] = ke::Move(data_[i - 1]);
    return true;
  }

  bool growIfNeeded(size_t needed)
  {
    if (!IsUintPtrAddSafe(nitems_, needed)) {
      this->reportAllocationOverflow();
      return false;
    }
    if (nitems_ + needed < maxsize_)
      return true;

    size_t new_maxsize = maxsize_ ? maxsize_ : 8;
    while (nitems_ + needed > new_maxsize) {
      if (!IsUintPtrMultiplySafe(new_maxsize, 2)) {
        this->reportAllocationOverflow();
        return false;
      }
      new_maxsize *= 2;
    }

    T* newdata = (T*)this->am_malloc(sizeof(T) * new_maxsize);
    if (newdata == nullptr)
      return false;
    MoveRange<T>(newdata, data_, nitems_);
    this->am_free(data_);

    data_ = newdata;
    maxsize_ = new_maxsize;
    return true;
  }

 private:
  T* data_;
  size_t nitems_;
  size_t maxsize_;
};

}

#endif /* _INCLUDE_KEIMA_TPL_CPP_VECTOR_H_ */

