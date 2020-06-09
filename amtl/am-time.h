// vim: set sts=4 ts=8 sw=4 tw=99 et:
//
// Copyright (C) 2020, David Anderson and AlliedModders LLC
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
#pragma once

// Support pre g++-4.8 is buggy.
#if __cplusplus >= 201703L

#include <limits.h>
#include <time.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace ke {

static inline constexpr std::chrono::nanoseconds 
TimespecToDuration(timespec ts)
{
    auto duration = std::chrono::seconds{ts.tv_sec} + std::chrono::nanoseconds{ts.tv_nsec};
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
}

template <typename Clock>
static inline constexpr std::chrono::time_point<Clock, std::chrono::nanoseconds>
TimespecToTimePoint(timespec ts)
{
    auto duration = TimespecToDuration(ts);
    return typename Clock::time_point{std::chrono::duration_cast<typename Clock::duration>(duration)};
}

// Uses the C++11 put_time() formatting, not strftime.
// See: https://en.cppreference.com/w/cpp/io/manip/put_time
static inline std::string FormatTime(const struct tm& tm, const char* fmt)
{
    std::ostringstream ss;
    ss << std::put_time(&tm, fmt);
    return ss.str();
}

static inline std::string FormatTime(const struct tm& tm, const std::string& str)
{
    return FormatTime(tm, str.c_str());
}

template <typename TimePointType, typename T>
static inline constexpr TimePointType EpochValueToTimePoint(T t)
{
    auto duration = typename TimePointType::duration{t};
    return TimePointType(duration);
}

} // namespace ke

#endif
