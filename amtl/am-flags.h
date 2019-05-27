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
#ifndef _include_amtl_flags_h_
#define _include_amtl_flags_h_

#include <amtl/am-enum.h>

namespace ke {

// This is a wrapper intended for use with typed/scoped enums in C++11. Since
// enum classes cannot be implicitly converted to bool, it makes patterns like
// |if (x & y)| possible without !!.
//
// Note that we explicitly don't expose an operator T here, since it creates
// ambiguous operator overloads if using KE_DEFINE_ENUM_OPERATORS.
template <typename T>
class Flags
{
  public:
    Flags()
     : value_(static_cast<T>(0))
    {}

    // Note: these are not explicit since we want operators below to support
    // both Flags and the underlying T type.
    Flags(const T& t)
     : value_(t)
    {}
    Flags(const Flags& other)
     : value_(other.value_)
    {}

    using IntType = typename enum_integral_type<T>::type;

    explicit operator bool() const {
        return value_ != static_cast<T>(0);
    }
    bool operator !() const {
        return value_ == static_cast<T>(0);
    }

    IntType bits() const {
        return static_cast<IntType>(value_);
    }
    T get() const {
        return value_;
    }

    Flags operator +(const Flags& other) const {
        return Flags(static_cast<T>(bits() | other.bits()));
    }
    Flags operator -(const Flags& other) const {
        return Flags(static_cast<T>(bits() & (bits() ^ other.bits())));
    }
    Flags operator |(const Flags& other) const {
        return Flags(static_cast<T>(bits() | other.bits()));
    }
    Flags operator &(const Flags& other) const {
        return Flags(static_cast<T>(bits() & other.bits()));
    }
    Flags operator ^(const Flags& other) const {
        return Flags(static_cast<T>(bits() ^ other.bits()));
    }
    bool operator ==(const Flags& other) const {
        return value_ == other.value_;
    }
    bool operator !=(const Flags& other) const {
        return value_ != other.value_;
    }

    Flags& operator +=(const Flags& other) {
        *this = *this + other;
        return *this;
    }
    Flags& operator -=(const Flags& other) {
        *this = *this - other;
        return *this;
    }
    Flags& operator |=(const Flags& other) {
        *this = *this | other;
        return *this;
    }
    Flags& operator &=(const Flags& other) {
        *this = *this & other;
        return *this;
    }
    Flags& operator ^=(const Flags& other) {
        *this = *this ^ other;
        return *this;
    }

    Flags& operator =(const Flags& other) {
        value_ = other.value_;
        return *this;
    }

  private:
    T value_;
};

} // namespace ke

#endif // _include_amtl_flags_h_
