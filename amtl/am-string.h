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

#ifndef _include_amtl_string_h_
#define _include_amtl_string_h_

#define __STDC_FORMAT_MACROS
#if !defined(KE_WINDOWS)
#    include <inttypes.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_MSC_VER)
#    include <strings.h>
#endif

#include <memory>
#include <utility>

#include <amtl/am-bits.h>
#include <amtl/am-cxx.h>
#include <amtl/am-platform.h>
#include <amtl/am-vector.h>

namespace ke {

#if defined(__GNUC__)
#    define KE_PRINTF_FUNCTION(string_index, first_to_check) \
        __attribute__((format(printf, string_index, first_to_check)))
#else
#    define KE_PRINTF_FUNCTION(string_index, first_to_check)
#endif

// ASCII string.
class AString
{
  public:
    AString()
     : length_(0)
    {}

    explicit AString(const char* str) {
        if (str && str[0]) {
            set(str, strlen(str));
        } else {
            chars_ = nullptr;
            length_ = 0;
        }
    }
    AString(const char* str, size_t length) {
        set(str, length);
    }
    AString(const AString& other) {
        if (other.length_)
            set(other.chars_.get(), other.length_);
        else
            length_ = 0;
    }
    AString(std::unique_ptr<char[]>&& ptr, size_t length)
     : chars_(std::move(ptr)),
       length_(length)
    {}
    AString(AString&& other)
     : chars_(std::move(other.chars_)),
       length_(other.length_)
    {
        other.length_ = 0;
    }

    AString& operator =(const char* str) {
        if (str && str[0]) {
            set(str, strlen(str));
        } else {
            chars_ = nullptr;
            length_ = 0;
        }
        return *this;
    }
    AString& operator =(const AString& other) {
        if (other.length_) {
            set(other.chars_.get(), other.length_);
        } else {
            chars_ = nullptr;
            length_ = 0;
        }
        return *this;
    }
    AString& operator =(AString&& other) {
        chars_ = std::move(other.chars_);
        length_ = other.length_;
        other.length_ = 0;
        return *this;
    }

    int compare(const char* str) const {
        return strcmp(chars(), str);
    }
    int compare(const AString& other) const {
        return strcmp(chars(), other.chars());
    }
    bool operator ==(const AString& other) const {
        return other.length() == length() && memcmp(other.chars(), chars(), length()) == 0;
    }

    char operator [](size_t index) const {
        assert(index < length());
        return chars()[index];
    }

    // Format a printf-style string and return nullptr on error.
    static inline std::unique_ptr<AString> Sprintf(const char* fmt, ...) KE_PRINTF_FUNCTION(1, 2);

    // Format a printf-style string and return nullptr on error.
    static inline std::unique_ptr<AString> SprintfArgs(const char* fmt, va_list ap)
        KE_PRINTF_FUNCTION(1, 0);

    // Format functions that work in-place.
    inline bool format(const char* fmt, ...) KE_PRINTF_FUNCTION(2, 3);
    inline bool formatArgs(const char* fmt, va_list ap) KE_PRINTF_FUNCTION(2, 0);

    // Split the string. The split argument must be a non-empty string.
    inline Vector<AString> split(const char* sep) const;

    size_t length() const {
        return length_;
    }
    const char* chars() const {
        if (!chars_)
            return "";
        return chars_.get();
    }

    bool startsWith(const char* other) const {
      return strncmp(chars(), other, strlen(other)) == 0;
    }

    AString uppercase() const {
        std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length_ + 1);
        for (size_t i = 0; i < length_; i++)
            buffer[i] = toupper(chars_[i]);
        buffer[length_] = '\0';
        return AString(std::move(buffer), length_);
    }
    AString lowercase() const {
        std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length_ + 1);
        for (size_t i = 0; i < length_; i++)
            buffer[i] = tolower(chars_[i]);
        buffer[length_] = '\0';
        return AString(std::move(buffer), length_);
    }

    bool empty() const {
        return length_ == 0;
    }

  private:
    static const size_t kInvalidLength = (size_t)-1;

    void set(const char* str, size_t length) {
        chars_ = std::make_unique<char[]>(length + 1);
        length_ = length;
        memcpy(chars_.get(), str, length);
        chars_[length] = '\0';
    }

  private:
    std::unique_ptr<char[]> chars_;
    size_t length_;
};

#if defined(_MSC_VER)
#    define KE_VSNPRINTF _vsnprintf
#else
#    define KE_VSNPRINTF vsnprintf
#endif

// Forward declare these functions since GCC will complain otherwise.
static inline std::unique_ptr<char[]> SprintfArgs(const char* fmt, va_list ap) KE_PRINTF_FUNCTION(1, 0);
static inline std::unique_ptr<char[]> Sprintf(const char* fmt, ...) KE_PRINTF_FUNCTION(1, 2);
static inline size_t SafeVsprintf(char* buffer, size_t maxlength, const char* fmt, va_list ap)
    KE_PRINTF_FUNCTION(3, 0);
static inline size_t SafeSprintf(char* buffer, size_t maxlength, const char* fmt, ...)
    KE_PRINTF_FUNCTION(3, 4);

namespace detail {

// Another forward declare thanks to GCC rules.
static inline std::unique_ptr<char[]> SprintfArgsImpl(size_t* out_len, const char* fmt, va_list ap)
    KE_PRINTF_FUNCTION(2, 0);

static inline std::unique_ptr<char[]>
SprintfArgsImpl(size_t* out_len, const char* fmt, va_list ap)
{
    va_list argv;
    va_copy(argv, ap);

    *out_len = 0;

#if defined(_MSC_VER)
    size_t len = _vscprintf(fmt, ap);
#else
    char tmp[2];
    size_t len = vsnprintf(tmp, sizeof(tmp), fmt, ap);
#endif

    std::unique_ptr<char[]> buffer;
    if (len == size_t(-1)) {
        va_end(argv);
        return buffer;
    }
    if (len == 0) {
        buffer = std::make_unique<char[]>(1);
        buffer[0] = '\0';
        va_end(argv);
        return buffer;
    }

    buffer = std::make_unique<char[]>(len + 1);
    if (!buffer) {
        va_end(argv);
        return buffer;
    }

    *out_len = KE_VSNPRINTF(buffer.get(), len + 1, fmt, argv);
    assert(*out_len == len);

    va_end(argv);
    return buffer;
}

} // namespace detail

static inline std::unique_ptr<char[]>
SprintfArgs(const char* fmt, va_list ap)
{
    size_t len;
    return detail::SprintfArgsImpl(&len, fmt, ap);
}

static inline std::unique_ptr<char[]>
Sprintf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::unique_ptr<char[]> result = SprintfArgs(fmt, ap);
    va_end(ap);
    return result;
}

inline std::unique_ptr<AString>
AString::SprintfArgs(const char* fmt, va_list ap)
{
    size_t len;
    std::unique_ptr<char[]> result = detail::SprintfArgsImpl(&len, fmt, ap);
    if (!result)
        return nullptr;
    return std::make_unique<AString>(std::move(result), len);
}

inline std::unique_ptr<AString>
AString::Sprintf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::unique_ptr<AString> result = AString::SprintfArgs(fmt, ap);
    va_end(ap);
    return result;
}

inline bool
AString::formatArgs(const char* fmt, va_list ap)
{
    size_t len;
    std::unique_ptr<char[]> result = detail::SprintfArgsImpl(&len, fmt, ap);
    if (!result) {
        *this = AString();
        return false;
    }
    *this = AString(std::move(result), len);
    return true;
}

inline bool
AString::format(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bool ok = AString::formatArgs(fmt, ap);
    va_end(ap);
    return ok;
}

static inline size_t
SafeVsprintf(char* buffer, size_t maxlength, const char* fmt, va_list ap)
{
    if (!maxlength)
        return 0;

    size_t len = KE_VSNPRINTF(buffer, maxlength, fmt, ap);

    if (len >= maxlength) {
        buffer[maxlength - 1] = '\0';
        return maxlength - 1;
    }

    return len;
}

static inline size_t
SafeSprintf(char* buffer, size_t maxlength, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    size_t len = SafeVsprintf(buffer, maxlength, fmt, ap);
    va_end(ap);
    return len;
}

static inline size_t
SafeStrcpy(char* dest, size_t maxlength, const char* src)
{
    if (!dest || !maxlength)
        return 0;

    char* iter = dest;
    size_t count = maxlength;
    while (*src && --count)
        *iter++ = *src++;
    *iter = '\0';

    return iter - dest;
}

// Append |src| to |dest|, up to |maxlength-1| characters to ensure a null
// terminator. The new string size is returned.
static inline size_t
SafeStrcat(char* dest, size_t maxlength, const char* src)
{
    char* iter = dest;
    while (*iter)
        iter++;

    // Note: add 1 to ensure we always have room for the null terminator.
    while (*src && (size_t(iter - dest) + 1 < maxlength))
        *iter++ = *src++;
    *iter = '\0';
    return iter - dest;
}

// Same as SafeStrcpy, but only copy up to n chars.
static inline size_t
SafeStrcpyN(char* dest, size_t maxlength, const char* src, size_t n)
{
    if (!dest || !maxlength || !n)
        return 0;

    char* iter = dest;
    size_t count = maxlength;
    while (--count && n--)
        *iter++ = *src++;
    *iter = '\0';

    return iter - dest;
}

static inline int
StrCaseCmp(const char* a, const char* b)
{
#if defined(_MSC_VER)
    return _strcmpi(a, b);
#else
    return strcasecmp(a, b);
#endif
}

// Split a string into a vector of strings. The |split| argument must contain
// a non-empty string. The operation is defined such that:
//
//   Join(Split(str, sep), sep) == str
//
static inline ke::Vector<AString>
Split(const char* str, const char* split)
{
    size_t split_len = strlen(split);
    assert(split_len > 0);

    Vector<AString> out;

    const char* cursor = str;
    const char* match = nullptr;
    while (*cursor) {
        match = strstr(cursor, split);
        if (!match)
            break;

        out.append(AString(cursor, match - cursor));
        cursor = match + split_len;
    }

    if (*cursor != '\0' || match)
        out.append(AString(cursor));
    return out;
}

// Given a list of strings, return a string combining them all with |sep|
// appended in between.
//
// Unlike Split(), |sep| can be an empty string.
static inline AString
Join(const Vector<AString>& pieces, const char* sep)
{
    size_t sep_len = strlen(sep);
    size_t buffer_len = 1;

    for (const AString& piece : pieces)
        buffer_len += piece.length();
    if (!pieces.empty())
        buffer_len += sep_len * (pieces.length() - 1);

    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(buffer_len);

    char* iter = buffer.get();
    char* end = buffer.get() + buffer_len;
    for (size_t i = 0; i < pieces.length(); i++) {
        SafeStrcpy(iter, end - iter, pieces[i].chars());
        iter += pieces[i].length();

        if (i != pieces.length() - 1) {
            SafeStrcpy(iter, end - iter, sep);
            iter += sep_len;
        }
    }
    *iter++ = '\0';
    assert(iter == end);

    return AString(std::move(buffer), buffer_len - 1);
}

inline Vector<AString>
AString::split(const char* sep) const
{
    return Split(chars(), sep);
}

#if defined(KE_WINDOWS)
#    define KE_FMT_SIZET "Iu"
#    define KE_FMT_I64 "I64d"
#    define KE_FMT_U64 "I64u"
#elif defined(KE_POSIX)
#    define KE_FMT_SIZET "zu"
#    define KE_FMT_I64 PRId64
#    define KE_FMT_U64 PRIu64
#else
#    error "Implement format specifier string"
#endif

} // namespace ke

#endif // _include_amtl_string_h_
