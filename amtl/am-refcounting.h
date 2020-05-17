// vim: set sts=8 ts=4 sw=4 tw=99 et:
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

#ifndef _include_amtl_refcounting_h_
#define _include_amtl_refcounting_h_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <amtl/am-raii.h>

namespace ke {

template <typename T>
class RefPtr;

// Objects in AMTL inheriting from Refcounted will have an initial refcount
// of 0. However, in some systems (such as COM), the initial refcount is 1,
// or functions may return raw pointers that have been AddRef'd. In these
// cases it would be a mistake to use RefPtr<> since the object leak an extra
// reference.
//
// This container holds a refcounted object without addrefing it. This is
// intended only for interacting with functions which return an object that
// has been manually AddRef'd. Note that this will perform a Release(), so
// so it is necessary to assign it to retain the object.
template <typename T>
class AlreadyRefed
{
  public:
    AlreadyRefed(decltype(nullptr))
     : thing_(nullptr)
    {}
    explicit AlreadyRefed(T* t)
     : thing_(t)
    {}
    AlreadyRefed(const AlreadyRefed<T>& other)
     : thing_(other.thing_)
    {
        if (thing_)
            thing_->AddRef();
    }
    AlreadyRefed(AlreadyRefed<T>&& other)
     : thing_(other.thing_)
    {
        other.thing_ = nullptr;
    }
    ~AlreadyRefed() {
        if (thing_)
            thing_->Release();
    }

    bool operator !() const {
        return !thing_;
    }
    T* operator ->() const {
        return thing_;
    }
    bool operator ==(T* other) const {
        return thing_ == other;
    }
    bool operator !=(T* other) const {
        return thing_ != other;
    }

    // Analagous to AutoPtr::take().
    T* take() const {
        return ReturnAndVoid(thing_);
    }

  private:
    mutable T* thing_;
};

template <typename T>
static inline AlreadyRefed<T>
AdoptRef(T* t)
{
    return AlreadyRefed<T>(t);
}

// Classes which are refcounted should inherit from this. Note that reference
// counts start at 0 in AMTL, rather than 1. This avoids the complexity of
// having to adopt the initial ref upon allocation. However, this also means
// invoking Release() on a newly allocated object is illegal. Newborn objects
// must either be assigned to a RefPtr or must be deleted using |delete|.
template <typename T>
class Refcounted
{
  public:
    Refcounted()
     : refcount_(0)
    {}

    void AddRef() {
        refcount_++;
    }
    void Release() {
        assert(refcount_ > 0);
        if (--refcount_ == 0)
            delete static_cast<T*>(this);
    }

  protected:
    ~Refcounted() {}

  private:
    uintptr_t refcount_;
};

// Use this to forward to ke::Refcounted<X>, when implementing IRefcounted.
#define KE_IMPL_REFCOUNTING(classname)        \
    void AddRef() {                           \
        ke::Refcounted<classname>::AddRef();  \
    }                                         \
    void Release() {                          \
        ke::Refcounted<classname>::Release(); \
    }

// This can be used for classes which will inherit from VirtualRefcounted.
class IRefcounted
{
  public:
    virtual ~IRefcounted() {}
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};

// Classes may be multiply-inherited may wish to derive from this Refcounted
// instead.
class VirtualRefcounted : public IRefcounted
{
  public:
    VirtualRefcounted()
     : refcount_(0)
    {
#if !defined(NDEBUG)
        destroying_ = false;
#endif
    }
    virtual ~VirtualRefcounted() {}
    void AddRef() override {
        assert(!destroying_);
        refcount_++;
    }
    void Release() override {
        assert(refcount_ > 0);
        if (--refcount_ == 0) {
#if !defined(NDEBUG)
            destroying_ = true;
#endif
            delete this;
        }
    }

  private:
    uintptr_t refcount_;
#if !defined(NDEBUG)
    bool destroying_;
#endif
};

// Simple class for automatic refcounting.
template <typename T>
class RefPtr
{
  public:
    RefPtr(T* thing)
     : thing_(thing)
    {
        AddRef();
    }

    RefPtr()
     : thing_(nullptr)
    {}

    RefPtr(const RefPtr& other)
     : thing_(other.thing_)
    {
        AddRef();
    }
    RefPtr(RefPtr&& other)
     : thing_(other.thing_)
    {
        other.thing_ = nullptr;
    }
    template <typename S>
    RefPtr(const RefPtr<S>& other)
     : thing_(*other)
    {
        AddRef();
    }
    template <typename S>
    RefPtr(RefPtr<S>&& other)
     : thing_(other.forget().take())
    {}
    RefPtr(const AlreadyRefed<T>& other)
     : thing_(other.take())
    {}
    template <typename S>
    RefPtr(const AlreadyRefed<S>& other)
     : thing_(other.take())
    {}
    ~RefPtr() {
        Release();
    }

    T* operator ->() const {
        return operator*();
    }
    T* operator *() const {
        return thing_;
    }
    operator T*() {
        return thing_;
    }
    operator T*() const {
        return thing_;
    }
    bool operator !() const {
        return !thing_;
    }
    operator T&() {
        return *thing_;
    }
    explicit operator bool() const {
        return !!thing_;
    }

    AlreadyRefed<T> take() {
        return AlreadyRefed<T>(ReturnAndVoid(thing_));
    }
    AlreadyRefed<T> forget() {
        return AlreadyRefed<T>(ReturnAndVoid(thing_));
    }

    template <typename S>
    RefPtr& operator =(S* thing) {
        Release();
        thing_ = thing;
        AddRef();
        return *this;
    }

    template <typename S>
    RefPtr& operator =(const AlreadyRefed<S>& other) {
        Release();
        thing_ = other.take();
        return *this;
    }

    RefPtr& operator =(const RefPtr& other) {
        Release();
        thing_ = other.thing_;
        AddRef();
        return *this;
    }

    RefPtr& operator =(RefPtr&& other) {
        Release();
        thing_ = other.thing_;
        other.thing_ = nullptr;
        return *this;
    }

    T* get() const {
        return thing_;
    }
    T** byref() {
        return &thing_;
    }
    T* const* byref_const() const {
        return &thing_;
    }
    void** address() {
        return reinterpret_cast<void**>(&thing_);
    }

  private:
    void AddRef() {
        if (thing_)
            thing_->AddRef();
    }
    void Release() {
        if (thing_)
            thing_->Release();
    }

  protected:
    T* thing_;
};

} // namespace ke

#endif // _include_amtl_refcounting_h_
