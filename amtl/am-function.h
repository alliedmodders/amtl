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

#include <amtl/am-uniqueptr.h>
#include <amtl/am-refcounting-threadsafe.h>

namespace ke {

template <typename T>
class Callable;

template <typename ReturnType, typename ...ParameterTypes>
class Callable<ReturnType(ParameterTypes...)>
{
protected:
  class Invoker : public RefcountedThreadsafe<Invoker>
  {
  protected:
    Invoker()
    {}
  public:
    Invoker(const Invoker&) = delete;
    Invoker(Invoker&&) = delete;
    virtual ~Invoker()
    {}

  public:
    virtual bool can_invoke() const = 0;
    virtual ReturnType invoke(ParameterTypes&&... parameters) const = 0;

  public:
    Invoker& operator=(const Invoker&) = delete;
    Invoker& operator=(Invoker&&) = delete;
  };

public:
  Callable() {
    assign(nullptr);
  }
  Callable(decltype(nullptr))
    : Callable()
  {}
  Callable(const Callable& other) {
    assign(other);
  }
  Callable(Callable&& other) {
    assign(Forward<Callable>(other));
  }
  ~Callable()
  {}

public:
  bool can_invoke() const {
    return !!invoker_ && invoker_->can_invoke();
  }
  ReturnType invoke(ParameterTypes&&... parameters) const {
    assert(can_invoke());
    return invoker_->invoke(Forward<ParameterTypes>(parameters)...);
  }
  void assign(const Callable& other) {
    invoker_ = other.invoker_;
  }
  void assign(Callable&& other) {
    invoker_ = Move(other.invoker_);
  }
protected:
  void assign(Invoker* invoker) {
    invoker_ = invoker;
  }

public:
  explicit operator bool() const {
    return can_invoke();
  }
  bool operator!() const {
    return !(bool)*this;
  }
  bool operator==(const Callable& other) {
    return *invoker_ == *other.invoker_;
  }
  bool operator!=(const Callable& other) {
    return !(*this == other);
  }
  ReturnType operator()(ParameterTypes&&... parameters) const {
    return invoke(Forward<ParameterTypes>(parameters)...);
  }
  Callable& operator=(decltype(nullptr)) {
    assign(nullptr);
    return *this;
  }
  Callable& operator=(const Callable& other) {
    assign(other);
    return *this;
  }
  Callable& operator=(Callable&& other) {
    assign(Forward<Callable>(other));
    return *this;
  }

private:
  RefPtr<Invoker> invoker_;
};

template <typename T>
class FunctionPointer;

template <typename ReturnType, typename ...ParameterTypes>
class FunctionPointer<ReturnType(ParameterTypes...)> : public Callable<ReturnType(ParameterTypes...)>
{
private:
  class CFunctionInvoker : public Callable<ReturnType(ParameterTypes...)>::Invoker
  {
  public:
    typedef ReturnType(*FunctionType)(ParameterTypes...);

  public:
    CFunctionInvoker(FunctionType function)
      : function_(function)
    {}

  public:
    virtual bool can_invoke() const override {
      return !!function_;
    }
    virtual ReturnType invoke(ParameterTypes&&... parameters) const override {
      return function_(Forward<ParameterTypes>(parameters)...);
    }

  private:
    FunctionType function_;
  };
  template <typename U>
  class MFunctionInvoker : public Callable<ReturnType(ParameterTypes...)>::Invoker
  {
  private:
    class Invoker
    {
    protected:
      Invoker()
      {}
    public:
      Invoker(const Invoker&) = delete;
      Invoker(Invoker&&) = delete;
      virtual ~Invoker()
      {}

    public:
      virtual bool has_function() const = 0;
      virtual ReturnType invoke(U* ptr, ParameterTypes&&... parameters) const = 0;

    public:
      Invoker& operator=(const Invoker&) = delete;
      Invoker& operator=(Invoker&&) = delete;
    };
    class MemberInvoker : public Invoker
    {
    public:
      typedef ReturnType(U::*FunctionType)(ParameterTypes...);

    public:
      MemberInvoker(FunctionType function)
        : function_(function)
      {}

    public:
      virtual bool has_function() const override {
        return !!function_;
      }
      virtual ReturnType invoke(U* ptr, ParameterTypes&&... parameters) const override {
        return (ptr->*function_)(Forward<ParameterTypes>(parameters)...);
      }

    private:
      FunctionType function_;
    };
    class CMemberInvoker : public Invoker
    {
    public:
      typedef ReturnType(U::*FunctionType)(ParameterTypes...) const;

    public:
      CMemberInvoker(FunctionType function)
        : function_(function)
      {}

    public:
      virtual bool has_function() const override {
        return !!function_;
      }
      virtual ReturnType invoke(U* ptr, ParameterTypes&&... parameters) const override {
        return (ptr->*function_)(Forward<ParameterTypes>(parameters)...);
      }

    private:
      FunctionType function_;
    };
    class VMemberInvoker : public Invoker
    {
    public:
      typedef ReturnType(U::*FunctionType)(ParameterTypes...) volatile;

    public:
      VMemberInvoker(FunctionType function)
        : function_(function)
      {}

    public:
      virtual bool has_function() const override {
        return !!function_;
      }
      virtual ReturnType invoke(U* ptr, ParameterTypes&&... parameters) const override {
        return (ptr->*function_)(Forward<ParameterTypes>(parameters)...);
      }

    private:
      FunctionType function_;
    };
    class CVMemberInvoker : public Invoker
    {
    public:
      typedef ReturnType(U::*FunctionType)(ParameterTypes...) const volatile;

    public:
      CVMemberInvoker(FunctionType function)
        : function_(function)
      {}

    public:
      virtual bool has_function() const override {
        return !!function_;
      }
      virtual ReturnType invoke(U* ptr, ParameterTypes&&... parameters) const override {
        return (ptr->*function_)(Forward<ParameterTypes>(parameters)...);
      }

    private:
      FunctionType function_;
    };

  public:
    typedef typename MemberInvoker::FunctionType MFunctionType;
    typedef typename CMemberInvoker::FunctionType CMFunctionType;
    typedef typename VMemberInvoker::FunctionType VMFunctionType;
    typedef typename CVMemberInvoker::FunctionType CVMFunctionType;

  public:
    MFunctionInvoker(U* ptr, MFunctionType function)
      : ptr_(ptr),
      invoker_(new MemberInvoker(function))
    {}
    MFunctionInvoker(U* ptr, CMFunctionType function)
      : ptr_(ptr),
      invoker_(new CMemberInvoker(function))
    {}
    MFunctionInvoker(U* ptr, VMFunctionType function)
      : ptr_(ptr),
      invoker_(new VMemberInvoker(function))
    {}
    MFunctionInvoker(U* ptr, CVMFunctionType function)
      : ptr_(ptr),
      invoker_(new CVMemberInvoker(function))
    {}

  public:
    virtual bool can_invoke() const override {
      return !!ptr_ && !!invoker_ && invoker_->has_function();
    }
    virtual ReturnType invoke(ParameterTypes&&... parameters) const override {
      return invoker_->invoke(ptr_, Forward<ParameterTypes>(parameters)...);
    }

  private:
    U* ptr_;
    UniquePtr<Invoker> invoker_;
  };

public:
  FunctionPointer()
    : Callable<ReturnType(ParameterTypes...)>()
  {}
  FunctionPointer(decltype(nullptr))
    : Callable<ReturnType(ParameterTypes...)>(nullptr)
  {}
  FunctionPointer(const FunctionPointer& other)
    : Callable<ReturnType(ParameterTypes...)>(other)
  {}
  FunctionPointer(FunctionPointer&& other)
    : Callable<ReturnType(ParameterTypes...)>(Forward<FunctionPointer>(other))
  {}
  FunctionPointer(typename CFunctionInvoker::FunctionType function) {
    assign(function);
  }
  template <typename U>
  FunctionPointer(U* ptr, typename MFunctionInvoker<U>::MFunctionType function) {
    assign(ptr, function);
  }
  template <typename U>
  FunctionPointer(U* ptr, typename MFunctionInvoker<U>::CMFunctionType function) {
    assign(ptr, function);
  }
  template <typename U>
  FunctionPointer(U* ptr, typename MFunctionInvoker<U>::VMFunctionType function) {
    assign(ptr, function);
  }
  template <typename U>
  FunctionPointer(U* ptr, typename MFunctionInvoker<U>::CVMFunctionType function) {
    assign(ptr, function);
  }
  template <typename U>
  FunctionPointer(U* ptr) {
    assign(ptr, &U::operator());
  }

public:
  FunctionPointer& operator=(decltype(nullptr)) {
    return (FunctionPointer&)Callable<ReturnType(ParameterTypes...)>::operator=(nullptr);
  }
  FunctionPointer& operator=(const FunctionPointer& other) {
    return (FunctionPointer&)Callable<ReturnType(ParameterTypes...)>::operator=(other);
  }
  FunctionPointer& operator=(FunctionPointer&& other) {
    return (FunctionPointer&)Callable<ReturnType(ParameterTypes...)>::operator=(Forward<FunctionPointer>(other));
  }
  FunctionPointer& operator=(typename CFunctionInvoker::FunctionType function) {
    assign(function);
    return *this;
  }
  template <typename U>
  FunctionPointer& operator=(U* ptr) {
    assign(ptr, &U::operator());
    return *this;
  }

public:
  template <typename U>
  void assign(U* ptr, typename MFunctionInvoker<U>::MFunctionType function) {
    Callable<ReturnType(ParameterTypes...)>::assign(new MFunctionInvoker<U>(ptr, function));
  }
  template <typename U>
  void assign(U* ptr, typename MFunctionInvoker<U>::CMFunctionType function) {
    Callable<ReturnType(ParameterTypes...)>::assign(new MFunctionInvoker<U>(ptr, function));
  }
  template <typename U>
  void assign(U* ptr, typename MFunctionInvoker<U>::VMFunctionType function) {
    Callable<ReturnType(ParameterTypes...)>::assign(new MFunctionInvoker<U>(ptr, function));
  }
  template <typename U>
  void assign(U* ptr, typename MFunctionInvoker<U>::CVMFunctionType function) {
    Callable<ReturnType(ParameterTypes...)>::assign(new MFunctionInvoker<U>(ptr, function));
  }
private:
  void assign(typename CFunctionInvoker::FunctionType function) {
    Callable<ReturnType(ParameterTypes...)>::assign(new CFunctionInvoker(function));
  }
};

template <typename T>
class Lambda;

template <typename ReturnType, typename ...ParameterTypes>
class Lambda<ReturnType(ParameterTypes...)> : public Callable<ReturnType(ParameterTypes...)>
{
private:
  template <typename U>
  class LambdaInvoker : public Callable<ReturnType(ParameterTypes...)>::Invoker
  {
  public:
    LambdaInvoker(const U& obj)
      : obj_(obj)
    {}
    LambdaInvoker(U&& obj)
      : obj_(Move(obj))
    {}

  public:
    virtual bool can_invoke() const override {
      return true; // This can never be false in our knowledge.
    }
    virtual ReturnType invoke(ParameterTypes&&... parameters) const override {
      return obj_(Forward<ParameterTypes>(parameters)...);
    }

  private:
    U obj_;
  };

public:
  Lambda()
    : Callable<ReturnType(ParameterTypes...)>()
  {}
  Lambda(decltype(nullptr))
    : Callable<ReturnType(ParameterTypes...)>(nullptr)
  {}
  Lambda(const Lambda& other)
    : Callable<ReturnType(ParameterTypes...)>(other)
  {}
  Lambda(Lambda&& other)
    : Callable<ReturnType(ParameterTypes...)>(Forward<Lambda>(other))
  {}
  template <typename U>
  Lambda(const U& obj) {
    assign(obj);
  }
  template <typename U>
  Lambda(U&& obj) {
    assign(Forward<U>(obj));
  }

public:
  Lambda& operator=(decltype(nullptr)) {
    return (Lambda&)Callable<ReturnType(ParameterTypes...)>::operator=(nullptr);
  }
  Lambda& operator=(const Lambda& other) {
    return (Lambda&)Callable<ReturnType(ParameterTypes...)>::operator=(other);
  }
  Lambda& operator=(Lambda&& other) {
    return (Lambda&)Callable<ReturnType(ParameterTypes...)>::operator=(Forward<Lambda>(other));
  }
  template <typename U>
  Lambda& operator=(const U& obj) {
    assign(obj);
    return *this;
  }
  template <typename U>
  Lambda& operator=(U&& obj) {
    assign(Forward<U>(obj));
    return *this;
  }

private:
  template <typename U>
  void assign(U&& obj) {
    typedef typename decay<U>::type CallableType;
    typedef LambdaInvoker<CallableType> HolderType;

    Callable<ReturnType(ParameterTypes...)>::assign(new HolderType(Forward<U>(obj)));
  }
};

} // namespace ke

#endif // _include_amtl_function_h_

