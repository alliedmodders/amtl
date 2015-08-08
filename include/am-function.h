// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013-2015, David Anderson and AlliedModders LLC
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above cloneright notice, this
//    list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above cloneright notice,
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
#ifndef _include_amtl_function_h_
#define _include_amtl_function_h_

#include <functional>
#include <am-moveable.h>
#include <am-type-traits.h>
#include <new>
#include <assert.h>

namespace ke {

namespace impl {
  template <typename ReturnType, typename ...ArgTypes>
  class FuncHolderBase
  {
   public:
     virtual ~FuncHolderBase()
     {}
     virtual ReturnType invoke(ArgTypes&&... argv) = 0;
     virtual FuncHolderBase<ReturnType, ArgTypes...>* clone(void* mem) = 0;
     virtual FuncHolderBase<ReturnType, ArgTypes...>* move(void* mem) = 0;
  };

  template <typename T, typename ReturnType, typename ...ArgTypes>
  class FuncHolder : public FuncHolderBase<ReturnType, ArgTypes...>
  {
    typedef FuncHolderBase<ReturnType, ArgTypes...> BaseType;

   public:
    FuncHolder(const FuncHolder& other)
     : obj_(other.obj_)
    {}
    FuncHolder(FuncHolder&& other)
     : obj_(ke::Move(other.obj_))
    {}
    FuncHolder(const T& obj)
     : obj_(obj)
    {}
    FuncHolder(T&& obj)
     : obj_(ke::Move(obj))
    {}

    virtual ~FuncHolder()
    {}
    virtual ReturnType invoke(ArgTypes&&... argv) override {
      return obj_(ke::Forward<ArgTypes>(argv)...);
    }
    virtual BaseType* clone(void* mem) override {
       if (!mem)
         return new FuncHolder(*this);
       new (mem) FuncHolder(*this);
       return (BaseType*)mem;
    }
    virtual BaseType* move(void* mem) override {
      new (mem) FuncHolder(ke::Move(*this));
      return (BaseType*)mem;
    }

   private:
    T obj_;
  };

  static const size_t kMinLambdaInlineBufferSize = sizeof(void*) * 3;
} // namespace impl

template <typename Tk>
class Lambda;

template <typename ReturnType, typename ...ArgTypes>
class Lambda<ReturnType(ArgTypes...)>
{
  typedef impl::FuncHolderBase<ReturnType, ArgTypes...> HolderType;

 public:
  Lambda()
   : impl_(nullptr)
  {}
  Lambda(decltype(nullptr))
   : impl_(nullptr)
  {}
  Lambda(const Lambda& other) {
    assign(other);
  }
  Lambda(Lambda&& other) {
    move(ke::Forward<Lambda>(other));
  }

  template <typename T>
  Lambda(T&& obj) {
    assign(ke::Forward<T>(obj));
  }

  ~Lambda() {
    destroy();
  }

  Lambda& operator =(decltype(nullptr)) {
    destroy();
    impl_ = nullptr;
    return *this;
  }
  Lambda& operator =(const Lambda& other) {
    destroy();
    assign(other);
    return *this;
  }
  Lambda& operator =(Lambda&& other) {
    destroy();
    move(other);
    return *this;
  }

  template <typename T>
  Lambda& operator =(T&& other) {
    destroy();
    assign(other);
    return *this;
  }

  explicit operator bool() const {
    return !!impl_;
  }

  ReturnType operator()(ArgTypes... argv) {
    assert(impl_);
    return impl_->invoke(ke::Forward<ArgTypes>(argv)...);
  }

  bool usingInlineStorage() const {
    return (void *)impl_ == &buffer_;
  }

 private:
  void destroy() {
    if (!impl_)
      return;

    if (usingInlineStorage())
      impl_->~HolderType();
    else
      delete impl_;
  }
  void zap() {
    destroy();
    impl_ = nullptr;
  }

  void* inline_buffer() {
    return &buffer_;
  }

  void assign(const Lambda& other) {
    if (!other)
      impl_ = nullptr;
    else if (other.usingInlineStorage())
      impl_ = other.impl_->clone(inline_buffer());
    else
      impl_ = other.impl_->clone(nullptr);
  }

  void move(Lambda&& other) {
    if (!other) {
      impl_ = nullptr;
    } else if (other.usingInlineStorage()) {
      impl_ = other.impl_->move(inline_buffer());
      other.zap();
    } else {
      impl_ = other.impl_;
      other.impl_ = nullptr;
    }
  }

  template <typename T>
  void assign(T&& obj) {
    typedef ke::decay<T>::type CallableType;
    typedef impl::FuncHolder<CallableType, ReturnType, ArgTypes...> ImplType;

    if (sizeof(ImplType) <= sizeof(buffer_)) {
      impl_ = reinterpret_cast<ImplType*>(inline_buffer());
      new (inline_buffer()) ImplType(ke::Forward<T>(obj));
    } else {
      impl_ = new ImplType(ke::Forward<T>(obj));
    }
  }

 private:
  HolderType* impl_;
  union {
    double alignment_;
    char alias_[impl::kMinLambdaInlineBufferSize];
  } buffer_;
};

} // namespace ke

#endif // _include_amtl_function_h_