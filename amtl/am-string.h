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
#include <string>
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
static inline std::string StringPrintf(const char* fmt, ...) KE_PRINTF_FUNCTION(1, 2);
static inline std::string StringPrintfVa(const char* fmt, va_list ap) KE_PRINTF_FUNCTION(1, 0);

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
    int len = _vscprintf(fmt, ap);
#else
    char tmp[2];
    // Note: GCC incorrectly errors on this code, so suppress the warning.
    int len = vsnprintf(tmp, sizeof(tmp), fmt, ap);
#endif

    if (len < 0) {
        va_end(argv);
        return std::unique_ptr<char[]>();
    }

    if (len == 0) {
        auto buffer = std::make_unique<char[]>(1);
        buffer[0] = '\0';
        va_end(argv);
        return buffer;
    }

    auto buffer = std::make_unique<char[]>(len + 1);
    if (!buffer) {
        va_end(argv);
        return buffer;
    }

    int new_len = KE_VSNPRINTF(buffer.get(), len + 1, fmt, argv);
    if (new_len < 0)
        return std::unique_ptr<char[]>();
    assert(len == new_len);

    *out_len = new_len;

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

static inline std::string
StringPrintfVa(const char* fmt, va_list ap)
{
    size_t len;
    std::unique_ptr<char[]> result = detail::SprintfArgsImpl(&len, fmt, ap);
    if (!result)
        return "";
    return std::string(result.get(), len);
}

static inline std::string
StringPrintf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string result = StringPrintfVa(fmt, ap);
    va_end(ap);
    return result;
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
static inline std::vector<std::string>
Split(const char* str, const char* split)
{
    size_t split_len = strlen(split);
    assert(split_len > 0);

    std::vector<std::string> out;

    const char* cursor = str;
    const char* match = nullptr;
    while (*cursor) {
        match = strstr(cursor, split);
        if (!match)
            break;

        out.emplace_back(cursor, match - cursor);
        cursor = match + split_len;
    }

    if (*cursor != '\0' || match)
        out.emplace_back(cursor);
    return out;
}

static inline std::vector<std::string>
Split(const std::string& str, const char* split)
{
    return ke::Split(str.c_str(), split);
}

// Given a list of strings, return a string combining them all with |sep|
// appended in between.
//
// Unlike Split(), |sep| can be an empty string.
static inline std::string
Join(const std::vector<std::string>& pieces, const char* sep)
{
    size_t sep_len = strlen(sep);
    size_t buffer_len = 0;

    for (const std::string& piece : pieces)
        buffer_len += piece.size();
    if (!pieces.empty())
        buffer_len += sep_len * (pieces.size() - 1);

    std::string buffer(buffer_len, '\0');

    char* iter = &buffer[0];
#if !defined(NDEBUG)
    char* end = iter + buffer_len;
#endif
    for (size_t i = 0; i < pieces.size(); i++) {
        memcpy(iter, pieces[i].c_str(), pieces[i].size());
        iter += pieces[i].size();

        if (i != pieces.size() - 1) {
            memcpy(iter, sep, sep_len);
            iter += sep_len;
        }
    }
    assert(iter == end);
    return buffer;
}

static inline std::string Uppercase(const char* str) {
    std::string result(str);
    for (size_t i = 0; i < result.size(); i++)
        result[i] = toupper(result[i]);
    return result;
}

static inline std::string Lowercase(const char* str) {
    std::string result(str);
    for (size_t i = 0; i < result.size(); i++)
        result[i] = tolower(result[i]);
    return result;
}

static inline bool StartsWith(const std::string& first, const char* other) {
    return strncmp(first.c_str(), other, strlen(other)) == 0;
}

static inline bool StartsWith(const char* first, const char* other) {
    return strncmp(first, other, strlen(other)) == 0;
}

static inline bool EndsWith(const std::string& first, const std::string& second) {
    return first.size() >= second.size() &&
           first.substr(first.size() - second.size(), second.size()) == second;
}

#if defined(KE_WINDOWS)
#    define KE_FMT_SIZET "Iu"
#    define KE_FMT_I64 "I64d"
#    define KE_FMT_U64 "I64u"
#    define KE_FMT_X64 "I64x"
#elif defined(KE_POSIX)
#    define KE_FMT_SIZET "zu"
#    define KE_FMT_I64 PRId64
#    define KE_FMT_U64 PRIu64
#    define KE_FMT_X64 PRIx64
#else
#    error "Implement format specifier string"
#endif

} // namespace ke

#endif // _include_amtl_string_h_
