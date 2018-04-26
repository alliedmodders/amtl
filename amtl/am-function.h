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

#include <amtl/am-moveable.h>
#include <amtl/am-type-traits.h>
#include <new>
#include <assert.h>

namespace ke {

namespace impl {
  template <typename ReturnType, typename ...ArgTypes>
  class FunctionHolderBase
  {
   public:
     virtual ~FunctionHolderBase()
     {}
     virtual ReturnType invoke(ArgTypes&&... argv) = 0;
     virtual FunctionHolderBase<ReturnType, ArgTypes...>* move(void* mem) = 0;
  };

  template <typename T, typename ReturnType, typename ...ArgTypes>
  class FunctionHolder : public FunctionHolderBase<ReturnType, ArgTypes...>
  {
    typedef FunctionHolderBase<ReturnType, ArgTypes...> BaseType;

   public:
    FunctionHolder(FunctionHolder&& other)
     : obj_(ke::Move(other.obj_))
    {}
    FunctionHolder(T&& obj)
     : obj_(ke::Move(obj))
    {}

    virtual ~FunctionHolder() override
    {}
    virtual ReturnType invoke(ArgTypes&&... argv) override {
      return obj_(ke::Forward<ArgTypes>(argv)...);
    }
    virtual BaseType* move(void* mem) override {
      new (mem) FunctionHolder(ke::Move(*this));
      return (BaseType*)mem;
    }

   private:
    T obj_;
  };

  static const size_t kMinFunctionInlineBufferSize = sizeof(void*) * 3;
} // namespace impl

template <typename Tk>
class Function;

template <typename ReturnType, typename ...ArgTypes>
class Function<ReturnType(ArgTypes...)>
{
  typedef impl::FunctionHolderBase<ReturnType, ArgTypes...> HolderType;

 public:
  Function()
   : impl_(nullptr)
  {}
  Function(decltype(nullptr))
   : impl_(nullptr)
  {}
  Function(Function&& other) {
    move(ke::Forward<Function>(other));
  }

  template <typename T>
  Function(T&& obj) {
    assign(ke::Forward<T>(obj));
  }

  ~Function() {
    destroy();
  }

  Function& operator =(decltype(nullptr)) {
    destroy();
    impl_ = nullptr;
    return *this;
  }
  Function& operator =(Function&& other) {
    destroy();
    move(ke::Move(other));
    return *this;
  }

  template <typename T>
  Function& operator =(T&& other) {
    destroy();
    assign(ke::Forward<T>(other));
    return *this;
  }

  explicit operator bool() const {
    return !!impl_;
  }

  ReturnType operator()(ArgTypes... argv) const {
    assert(impl_);
    return impl_->invoke(ke::Forward<ArgTypes>(argv)...);
  }

  bool usingInlineStorage() const {
    return (void*)impl_ == &buffer_;
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

  void move(Function&& other) {
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
    typedef typename ke::decay<T>::type CallableType;
    typedef impl::FunctionHolder<CallableType, ReturnType, ArgTypes...> ImplType;

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
    char alias_[impl::kMinFunctionInlineBufferSize];
  } buffer_;
};

namespace impl {
  template <typename ReturnType, typename ...ArgTypes>
  class LambdaHolderBase
  {
   public:
     virtual ~LambdaHolderBase()
     {}
     virtual ReturnType invoke(ArgTypes&&... argv) const = 0;
     virtual LambdaHolderBase<ReturnType, ArgTypes...>* clone(void* mem) const = 0;
     virtual LambdaHolderBase<ReturnType, ArgTypes...>* move(void* mem) = 0;
  };

  template <typename T, typename ReturnType, typename ...ArgTypes>
  class LambdaHolder : public LambdaHolderBase<ReturnType, ArgTypes...>
  {
    typedef LambdaHolderBase<ReturnType, ArgTypes...> BaseType;

   public:
    LambdaHolder(const LambdaHolder& other)
     : obj_(other.obj_)
    {}
    LambdaHolder(LambdaHolder&& other)
     : obj_(ke::Move(other.obj_))
    {}
    LambdaHolder(const T& obj)
     : obj_(obj)
    {}
    LambdaHolder(T&& obj)
     : obj_(ke::Move(obj))
    {}

    virtual ~LambdaHolder()
    {}
    virtual ReturnType invoke(ArgTypes&&... argv) const override {
      return obj_(ke::Forward<ArgTypes>(argv)...);
    }
    virtual BaseType* clone(void* mem) const override {
       if (!mem)
         return new LambdaHolder(*this);
       new (mem) LambdaHolder(*this);
       return (BaseType*)mem;
    }
    virtual BaseType* move(void* mem) override {
      new (mem) LambdaHolder(ke::Move(*this));
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
  typedef impl::LambdaHolderBase<ReturnType, ArgTypes...> HolderType;

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
    move(ke::Move(other));
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

  ReturnType operator()(ArgTypes... argv) const {
    assert(impl_);
    return impl_->invoke(ke::Forward<ArgTypes>(argv)...);
  }

  bool usingInlineStorage() const {
    return (void*)impl_ == &buffer_;
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
    typedef typename ke::decay<T>::type CallableType;
    typedef impl::LambdaHolder<CallableType, ReturnType, ArgTypes...> ImplType;

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

template <typename Tk>
class FuncPtr;

template <typename ReturnType, typename ...ArgTypes>
class FuncPtr<ReturnType(ArgTypes...)>
{
  typedef ReturnType(*Invoker)(void*, ArgTypes&&...);

 public:
  FuncPtr()
   : ptr_(nullptr)
  {}
  FuncPtr(const FuncPtr& other)
   : ptr_(other.ptr_),
     invoker_(other.invoker_)
  {}
  FuncPtr(ReturnType(*fn)(ArgTypes...)) {
    assignStatic(fn);
  }
  template <typename T>
  FuncPtr(T* obj) {
    assignFunctor(obj);
  }

  FuncPtr& operator =(decltype(nullptr)) {
    ptr_ = nullptr;
    invoker_ = nullptr;
    return *this;
  }
  FuncPtr& operator =(const FuncPtr& other) {
    ptr_ = other.ptr_;
    invoker_ = other.invoker_;
    return *this;
  }
  FuncPtr& operator =(ReturnType(*fn)(ArgTypes...)) {
    assignStatic(fn);
    return *this;
  }
  template <typename T>
  FuncPtr& operator =(T* obj) {
    assignFunctor(obj);
    return *this;
  }

  explicit operator bool() const {
    return !!ptr_;
  }

  ReturnType operator()(ArgTypes... argv) const {
    assert(ptr_ && invoker_);
    return invoker_(ptr_, ke::Forward<ArgTypes>(argv)...);
  }

 private:
  void assignStatic(ReturnType(*fn)(ArgTypes...)) {
    typedef decltype(fn) FnType;
    ptr_ = reinterpret_cast<void*>(fn);
    invoker_ = [](void* ptr, ArgTypes&&... argv) {
      return (reinterpret_cast<FnType>(ptr))(ke::Forward<ArgTypes>(argv)...);
    };
  }
  template <typename T>
  void assignFunctor(T* obj) {
    ptr_ = obj;
    invoker_ = [](void* ptr, ArgTypes&&... argv) {
      return (reinterpret_cast<T*>(ptr))->operator()(ke::Forward<ArgTypes>(argv)...);
    };
  }

 private:
  void* ptr_;
  Invoker invoker_;
};

} // namespace ke

#endif // _include_amtl_function_h_
