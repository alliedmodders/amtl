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

#ifndef _include_amtl_am_bits_h_
#define _include_amtl_am_bits_h_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER)
# include <intrin.h>
#endif
#include <amtl/am-cxx.h>
#include <amtl/am-platform.h>

namespace ke {

static const size_t kMallocAlignment = sizeof(void *) * 2;

static const size_t kKB = 1024;
static const size_t kMB = 1024 * kKB;
static const size_t kGB = 1024 * kMB;

#if defined(_MSC_VER)
template <typename T>
static uint32_t Log2(T value);

template <> inline uint32_t
Log2<uint32_t>(uint32_t number)
{
  assert(number != 0);

  unsigned long rval;
  _BitScanReverse(&rval, number);
  return rval;
}

template <> inline uint32_t
Log2<uint64_t>(uint64_t number)
{
  assert(number != 0);

  unsigned long rval;
#if defined(_M_X64)
  _BitScanReverse64(&rval, number);
#else
  if (number > uint32_t(0xFFFFFFFF)) {
    _BitScanReverse(&rval, uint32_t(number >> 32));
    rval += 32;
  } else {
    _BitScanReverse(&rval, uint32_t(number));
  }
#endif
  return rval;
}
#else
static inline size_t
Log2(size_t number)
{
  assert(number != 0);

#ifdef __GNUC__
  return 31 - __builtin_clz(number);
#else
  size_t bit;
  asm("bsr %1, %0\n"
      : "=r" (bit)
      : "rm" (number));
  return bit;
#endif
}
#endif

static inline size_t
FindRightmostBit(size_t number)
{
    assert(number != 0);

#ifdef _MSC_VER
    unsigned long rval;
# ifdef _M_IX86
    _BitScanForward(&rval, number);
# elif _M_X64
    _BitScanForward64(&rval, number);
# endif
    return rval;
#elif __GNUC__
    return __builtin_ctz(number);
#else
    size_t bit;
    asm("bsf %1, %0\n"
        : "=r" (bit)
        : "rm" (number));
    return bit;
#endif
}

static inline bool
IsPowerOfTwo(size_t value)
{
    if (value == 0)
        return false;
    return !(value & (value - 1));
}

static inline size_t
Align(size_t count, size_t alignment)
{
    assert(IsPowerOfTwo(alignment));
    return count + (alignment - (count % alignment)) % alignment;
}

static inline bool
IsUint32AddSafe(unsigned a, unsigned b)
{
    if (!a || !b)
        return true;
    size_t log2_a = Log2(a);
    size_t log2_b = Log2(b);
    return (log2_a < sizeof(unsigned) * 8) &&
           (log2_b < sizeof(unsigned) * 8);
}

static inline bool
IsUintPtrAddSafe(size_t a, size_t b)
{
    if (!a || !b)
        return true;
    size_t log2_a = Log2(a);
    size_t log2_b = Log2(b);
    return (log2_a < sizeof(size_t) * 8) &&
           (log2_b < sizeof(size_t) * 8);
}

static inline bool
IsUint32MultiplySafe(unsigned a, unsigned b)
{
    if (a <= 1 || b <= 1)
        return true;

    size_t log2_a = Log2(a);
    size_t log2_b = Log2(b);
    return log2_a + log2_b <= sizeof(unsigned) * 8;
}

static inline bool
IsUintPtrMultiplySafe(size_t a, size_t b)
{
    if (a <= 1 || b <= 1)
        return true;

    size_t log2_a = Log2(a);
    size_t log2_b = Log2(b);
    return log2_a + log2_b <= sizeof(size_t) * 8;
}

template <typename T>
static inline bool
IsAligned(T addr, size_t alignment)
{
    assert(IsPowerOfTwo(alignment));
    return !(uintptr_t(addr) & (alignment - 1));
}

static inline void *
AlignedBase(void *addr, size_t alignment)
{
    assert(IsPowerOfTwo(alignment));
    return reinterpret_cast<void *>(uintptr_t(addr) & ~(alignment - 1));
}

} // namespace ke

#endif // _include_amtl_am_bits_h_
