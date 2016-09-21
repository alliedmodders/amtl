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

#ifndef _include_amtl_string_h_
#define _include_amtl_string_h_

#define __STDC_FORMAT_MACROS
#if !defined(KE_WINDOWS)
# include <inttypes.h>
#endif
#include <amtl/am-autoptr.h>
#include <amtl/am-cxx.h>
#include <amtl/am-moveable.h>
#include <amtl/am-platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

namespace ke {

// ASCII string.
class AString
{
 public:
  AString()
   : length_(0)
  {
  }

  explicit AString(const char *str) {
    if (str && str[0]) {
      set(str, strlen(str));
    } else {
      chars_ = nullptr;
      length_ = 0;
    }
  }
  AString(const char *str, size_t length) {
    set(str, length);
  }
  AString(const AString &other) {
    if (other.length_)
      set(other.chars_.get(), other.length_);
    else
      length_ = 0;
  }
  AString(AString &&other)
    : chars_(other.chars_.take()),
      length_(other.length_)
  {
    other.length_ = 0;
  }

  AString &operator =(const char *str) {
    if (str && str[0]) {
      set(str, strlen(str));
    } else {
      chars_ = nullptr;
      length_ = 0;
    }
    return *this;
  }
  AString &operator =(const AString &other) {
    if (other.length_) {
      set(other.chars_.get(), other.length_);
    } else {
      chars_ = nullptr;
      length_ = 0;
    }
    return *this;
  }
  AString &operator =(AString &&other) {
    chars_ = Move(other.chars_);
    length_ = other.length_;
    other.length_ = 0;
    return *this;
  }

  int compare(const char *str) const {
    return strcmp(chars(), str);
  }
  int compare(const AString &other) const {
    return strcmp(chars(), other.chars());
  }
  bool operator ==(const AString &other) const {
    return other.length() == length() &&
           memcmp(other.chars(), chars(), length()) == 0;
  }

  char operator [](size_t index) const {
    assert(index < length());
    return chars()[index];
  }

  size_t length() const {
    return length_;
  }

  const char *chars() const {
    if (!chars_)
      return "";
    return chars_.get();
  }

 private:
  static const size_t kInvalidLength = (size_t)-1;

  void set(const char *str, size_t length) {
    chars_.assign(new char[length + 1]);
    length_ = length;
    memcpy(chars_.get(), str, length);
    chars_[length] = '\0';
  }

 private:
  AutoPtr<char[]> chars_;
  size_t length_;
};

static inline size_t
SafeVsprintf(char *buffer, size_t maxlength, const char *fmt, va_list ap)
{
  if (!maxlength)
    return 0;

  size_t len =
#if defined(KE_WINDOWS)
    _vsnprintf(buffer, maxlength, fmt, ap)
#else
    vsnprintf(buffer, maxlength, fmt, ap)
#endif
    ;

  if (len >= maxlength) {
    buffer[maxlength - 1] = '\0';
    return maxlength - 1;
  }

  return len;
}

static inline size_t
SafeSprintf(char *buffer, size_t maxlength, const char *fmt, ...)
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

#if defined(KE_WINDOWS)
# define KE_FMT_SIZET           "Iu"
# define KE_FMT_I64             "I64d"
# define KE_FMT_U64             "I64u"
#elif defined(KE_POSIX)
# define KE_FMT_SIZET           "zu"
# define KE_FMT_I64             PRId64
# define KE_FMT_U64             PRIu64
#else
# error "Implement format specifier string"
#endif

}

#endif // _include_amtl_string_h_
