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
#ifndef _include_amtl_maybe_h_
#define _include_amtl_maybe_h_

#include <amtl/am-moveable.h>
#include <amtl/am-storagebuffer.h>
#include <amtl/am-type-traits.h>
#include <assert.h>

namespace ke {

struct Nothing {};

template <typename T>
class Maybe
{
  public:
    Maybe()
     : initialized_(false)
    {}
    Maybe(const Maybe& other)
     : initialized_(false)
    {
        copyFrom(other);
    }
    Maybe(Maybe&& other)
     : initialized_(false)
    {
        moveFrom(ke::Move(other));
    }
    Maybe(Nothing)
     : initialized_(false)
    {}

    ~Maybe() {
        if (isValid())
            t_.address()->~T();
    }

    template <typename... ArgTypes>
    void init(ArgTypes&&... argv) {
        if (isValid())
            t_.address()->~T();

        new (t_.address()) T(Forward<ArgTypes>(argv)...);
        initialized_ = true;
    }

    bool isValid() const {
        return initialized_;
    }

    T& operator ->() {
        assert(isValid());
        return t_.address();
    }
    T& operator *() {
        assert(isValid());
        return *t_.address();
    }
    const T& operator ->() const {
        assert(isValid());
        return t_.address();
    }
    const T& operator *() const {
        assert(isValid());
        return *t_.address();
    }
    T& get() {
        assert(isValid());
        return *t_.address();
    }
    const T& get() const {
        assert(isValid());
        return *t_.address();
    }

    explicit operator bool() const {
        return isValid();
    }

    Maybe& operator =(const Maybe& other) {
        initialized_ = false;
        copyFrom(other);
        return *this;
    }
    Maybe& operator =(Maybe&& other) {
        initialized_ = false;
        moveFrom(ke::Move(other));
        return *this;
    }

  private:
    void copyFrom(const Maybe& other) {
        if (other.initialized_) {
            init(*other.t_.address());
        }
    }
    void moveFrom(Maybe&& other) {
        if (other.initialized_) {
            init(ke::Move(*other.t_.address()));
            other.initialized_ = false;
        }
    }

  private:
    bool initialized_;
    StorageBuffer<T> t_;
};

template <typename T, typename U = typename remove_cv<typename remove_reference<T>::type>::type>
static inline Maybe<U>
Some(T&& value)
{
    Maybe<U> m;
    m.init(ke::Forward<T>(value));
    return m;
}

} // namespace ke

#endif // _include_amtl_maybe_h_
