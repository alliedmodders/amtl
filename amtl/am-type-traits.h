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

#ifndef _include_amtl_type_traits_h_
#define _include_amtl_type_traits_h_

#include <amtl/am-cxx.h>
#include <stddef.h>

namespace ke {

// Remove references from types.
template <typename T>
struct remove_reference {
  typedef T type;
};
template <typename T>
struct remove_reference<T &> {
  typedef T type;
};
template <typename T>
struct remove_reference<T &&> {
  typedef T type;
};

template <typename T, T Value>
struct integral_constant {
  static const T value = Value;
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template<class T> struct is_lvalue_reference : false_type{};
template<class T> struct is_lvalue_reference<T&> : true_type {};

template <bool Value, typename True, typename False>
struct conditional {
  typedef True type;
};

template <typename True, typename False>
struct conditional<false, True, False> {
  typedef False type;
};

template <typename T>
struct is_array : false_type{};
template <typename T>
struct is_array<T[]> : true_type{};
template <typename T, size_t N>
struct is_array<T[N]> : true_type{};

template <typename T>
struct is_pointer : false_type{};
template <typename T>
struct is_pointer<T*> : true_type{};

template <typename T>
struct rm_rank {
  typedef T type;
};
template <typename T>
struct rm_rank<T[]> {
  typedef T type;
};
template <typename T, size_t N>
struct rm_rank<T[N]> {
  typedef T type;
};

template <typename T>
struct remove_volatile {
  typedef T type;
};
template <typename T>
struct remove_volatile<T volatile> {
  typedef T type;
};
template <typename T>
struct remove_const {
  typedef T type;
};
template <typename T>
struct remove_const<T const> {
  typedef T type;
};
template <typename T>
struct remove_cv {
  typedef typename remove_volatile<typename remove_const<T>::type>::type type;
};

template <typename T>
struct is_function_ptr : false_type{};
#if defined(_MSC_VER)
template <typename ReturnType, typename ...ArgTypes>
struct is_function_ptr<ReturnType (__cdecl *)(ArgTypes...)> : true_type{};
template <typename ReturnType, typename ...ArgTypes>
struct is_function_ptr<ReturnType (__cdecl *)(ArgTypes..., ...)> : true_type{};
#else
template <typename ReturnType, typename ...ArgTypes>
struct is_function_ptr<ReturnType (*)(ArgTypes...)> : true_type{};
template <typename ReturnType, typename ...ArgTypes>
struct is_function_ptr<ReturnType (*)(ArgTypes..., ...)> : true_type{};
#endif

template <typename T>
struct is_function : public is_function_ptr<typename remove_cv<T>::type *>
{};

template <typename T>
struct decay {
  typedef typename remove_reference<T>::type RvalType;
  typedef typename conditional<
    is_array<RvalType>::value,
    typename rm_rank<RvalType>::type*,
    typename conditional<
      is_function<RvalType>::value,
      RvalType*,
      typename remove_cv<RvalType>::type
    >::type
  >::type type;
};

// Remove one extent from an array type.
template <typename T>
struct remove_extent { typedef T type; };

template <typename T>
struct remove_extent<T[]> { typedef T type; };

template <typename T, size_t N>
struct remove_extent<T[N]> { typedef T type; };

} // namespace ke

#endif // _include_amtl_type_traits_h_
