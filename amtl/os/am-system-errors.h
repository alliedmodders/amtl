// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013-2015, David Anderson and AlliedModders LLC
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
#ifndef _include_amtl_os_system_errors_h_
#define _include_amtl_os_system_errors_h_

#if defined(KE_WINDOWS)
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#else
#    include <errno.h>
#endif
#include <amtl/am-string.h>
#include <string.h>

namespace ke {

static inline void
FormatSystemErrorCode(int code, char* error, size_t maxlength) {
#if defined(KE_WINDOWS)
    if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, code,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error, maxlength, nullptr)) {
        SafeSprintf(error, maxlength, "error code %08x", code);
        return;
    }
#elif defined(KE_LINUX) && defined(__GLIBC__)
    const char* ptr = strerror_r(code, error, maxlength);
    if (ptr != error)
        ke::SafeSprintf(error, maxlength, "%s", ptr);
#else
    strerror_r(code, error, maxlength);
#endif
}

static inline void
FormatSystemError(char* error, size_t maxlength) {
#if defined(KE_WINDOWS)
    FormatSystemErrorCode(GetLastError(), error, maxlength);
#else
    FormatSystemErrorCode(errno, error, maxlength);
#endif
}

} // namespace ke

#endif // _include_amtl_os_system_errors_h_
