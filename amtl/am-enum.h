// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2014, David Anderson and AlliedModders LLC
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
#ifndef _include_amtl_enum_h_
#define _include_amtl_enum_h_

#ifdef KE_ALLOW_STD_CXX
#    include <type_traits>
#endif

namespace ke {
template <typename T>
struct enum_integral_type {
#ifdef KE_ALLOW_STD_CXX
    using type = std::underlying_type<T>::type;
#else
    using type = __underlying_type(T);
#endif
};
} // namespace ke

#define KE_DEFINE_ENUM_OPERATORS(EnumName)                                          \
    static inline EnumName operator|(const EnumName& left, const EnumName& right) { \
        typedef ke::enum_integral_type<EnumName>::type int_type;                    \
        return EnumName(int_type(left) | int_type(right));                          \
    }                                                                               \
    static inline EnumName operator&(const EnumName& left, const EnumName& right) { \
        typedef ke::enum_integral_type<EnumName>::type int_type;                    \
        return EnumName(int_type(left) & int_type(right));                          \
    }                                                                               \
    static inline EnumName operator^(const EnumName& left, const EnumName& right) { \
        typedef ke::enum_integral_type<EnumName>::type int_type;                    \
        return EnumName(int_type(left) ^ int_type(right));                          \
    }                                                                               \
    static inline EnumName operator~(const EnumName& flags) {                       \
        typedef ke::enum_integral_type<EnumName>::type int_type;                    \
        return EnumName(~int_type(flags));                                          \
    }                                                                               \
    static inline EnumName& operator|=(EnumName& left, const EnumName& right) {     \
        return left = left | right;                                                 \
    }                                                                               \
    static inline EnumName& operator&=(EnumName& left, const EnumName& right) {     \
        return left = left & right;                                                 \
    }                                                                               \
    static inline EnumName& operator^=(EnumName& left, const EnumName& right) {     \
        return left = left ^ right;                                                 \
    }                                                                               \
    static inline bool operator!(const EnumName& obj) {                             \
        typedef ke::enum_integral_type<EnumName>::type int_type;                    \
        return int_type(obj) == 0;                                                  \
    }

#define KE_DEFINE_ENUM_COMPARATORS(EnumName, Type)                           \
    static inline bool operator==(const EnumName& left, const Type& right) { \
        return Type(left) == right;                                          \
    }                                                                        \
    static inline bool operator==(const Type& left, const EnumName& right) { \
        return left == Type(right);                                          \
    }                                                                        \
    static inline bool operator!=(const EnumName& left, const Type& right) { \
        return Type(left) != right;                                          \
    }                                                                        \
    static inline bool operator!=(const Type& left, const EnumName& right) { \
        return left != Type(right);                                          \
    }

#endif // _include_amtl_enum_h_
