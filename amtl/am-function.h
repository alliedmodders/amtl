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

#include <assert.h>
#include <stddef.h>

#include <new>
#include <utility>

namespace ke {

namespace impl {
template <typename ReturnType, typename... ArgTypes>
class FunctionHolderBase
{
  public:
    virtual ~FunctionHolderBase() {}
    virtual ReturnType invoke(ArgTypes&&... argv) const = 0;
    virtual FunctionHolderBase<ReturnType, ArgTypes...>* clone(void* mem) const = 0;
    virtual FunctionHolderBase<ReturnType, ArgTypes...>* move(void* mem) = 0;
};

template <typename T, typename ReturnType, typename... ArgTypes>
class FunctionHolder : public FunctionHolderBase<ReturnType, ArgTypes...>
{
    typedef FunctionHolderBase<ReturnType, ArgTypes...> BaseType;

  public:
    FunctionHolder(const FunctionHolder& other)
     : obj_(other.obj_)
    {}
    FunctionHolder(FunctionHolder&& other)
     : obj_(std::move(other.obj_))
    {}
    FunctionHolder(const T& obj)
     : obj_(obj)
    {}
    FunctionHolder(T&& obj)
     : obj_(std::move(obj))
    {}

    virtual ~FunctionHolder() {}
    virtual ReturnType invoke(ArgTypes&&... argv) const override {
        return obj_(std::forward<ArgTypes>(argv)...);
    }
    virtual BaseType* clone(void* mem) const override {
        if (!mem)
            return new FunctionHolder(*this);
        new (mem) FunctionHolder(*this);
        return (BaseType*)mem;
    }
    virtual BaseType* move(void* mem) override {
        new (mem) FunctionHolder(std::move(*this));
        return (BaseType*)mem;
    }

  private:
    T obj_;
};

static const size_t kMinFunctionInlineBufferSize = sizeof(void*) * 3;
} // namespace impl

template <typename Tk>
class Function;

template <typename ReturnType, typename... ArgTypes>
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
    Function(const Function& other) {
        assign(other);
    }
    Function(Function&& other) {
        move(std::forward<Function>(other));
    }

    template <typename T>
    Function(T&& obj) {
        assign(std::forward<T>(obj));
    }

    ~Function() {
        destroy();
    }

    Function& operator =(decltype(nullptr)) {
        destroy();
        impl_ = nullptr;
        return *this;
    }
    Function& operator =(const Function& other) {
        destroy();
        assign(other);
        return *this;
    }
    Function& operator =(Function&& other) {
        destroy();
        move(std::move(other));
        return *this;
    }

    template <typename T>
    Function& operator =(T&& other) {
        destroy();
        assign(other);
        return *this;
    }

    explicit operator bool() const {
        return !!impl_;
    }

    ReturnType operator ()(ArgTypes... argv) const {
        assert(impl_);
        return impl_->invoke(std::forward<ArgTypes>(argv)...);
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

    void assign(const Function& other) {
        if (!other)
            impl_ = nullptr;
        else if (other.usingInlineStorage())
            impl_ = other.impl_->clone(inline_buffer());
        else
            impl_ = other.impl_->clone(nullptr);
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
        typedef typename std::decay<T>::type CallableType;
        typedef impl::FunctionHolder<CallableType, ReturnType, ArgTypes...> ImplType;

        if (sizeof(ImplType) <= sizeof(buffer_)) {
            impl_ = reinterpret_cast<ImplType*>(inline_buffer());
            new (inline_buffer()) ImplType(std::forward<T>(obj));
        } else {
            impl_ = new ImplType(std::forward<T>(obj));
        }
    }

  private:
    HolderType* impl_;
    union {
        double alignment_;
        char alias_[impl::kMinFunctionInlineBufferSize];
    } buffer_;
};

} // namespace ke

#endif // _include_amtl_function_h_
