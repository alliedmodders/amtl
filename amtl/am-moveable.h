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

#ifndef _include_amtl_moveable_h_
#define _include_amtl_moveable_h_

#include <am-type-traits.h>

namespace ke {

// Previously, we implemented Move semantics without C++11. Now that we use
// C++11, we implement this as STL does for std::move.
template <typename T>
static inline typename remove_reference<T>::type &&
Move(T &&t)
{
  return static_cast<typename remove_reference<T>::type &&>(t);
}

// std::forward replacement. See:
//   http://thbecker.net/articles/rvalue_references/section_07.html and
//   http://thbecker.net/articles/rvalue_references/section_08.html
template <typename T>
static KE_CONSTEXPR inline T &&
Forward(typename remove_reference<T>::type &t) KE_NOEXCEPT
{
  return static_cast<T &&>(t);
}

template <typename T>
static KE_CONSTEXPR inline T &&
Forward(typename remove_reference<T>::type &&t) KE_NOEXCEPT
{
  return static_cast<T &&>(t);
}

template <typename T>
static inline void
MoveRange(T *dest, T *src, size_t length)
{
  for (size_t i = 0; i < length; i++) {
    new (&dest[i]) T(ke::Move(src[i]));
    src[i].~T();
  }
}

} // namespace ke

#endif // _include_amtl_moveable_h_
