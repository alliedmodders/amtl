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
#ifndef _include_amtl_algorithm_h_
#define _include_amtl_algorithm_h_

#include <amtl/am-cxx.h>
#include <amtl/am-moveable.h>

namespace ke {

template <typename T> static inline T
Min(const T& t1, const T& t2)
{
    return t1 < t2 ? t1 : t2;
}

template <typename T> static inline T
Max(const T& t1, const T& t2)
{
    return t1 > t2 ? t1 : t2;
}

template <typename T> static inline void
Swap(T& left, T& right)
{
  T tmp(Move(left));
  left = Move(right);
  right = Move(tmp);
}

template <typename T>
struct LessThan
{
  KE_CONSTEXPR bool operator ()(const T& left, const T& right) const {
    return left < right;
  }
};

template <typename T>
struct GreaterThan
{
  KE_CONSTEXPR bool operator ()(const T& left, const T& right) const {
    return left > right;
  }
};

} // namespace ke

#endif // _include_amtl_algorithm_h_
