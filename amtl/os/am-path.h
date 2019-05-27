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
#ifndef _include_amtl_os_path_h_
#define _include_amtl_os_path_h_

#include <amtl/am-platform.h>
#include <amtl/am-string.h>

namespace ke {
namespace path {

static inline size_t
FormatVa(char* dest, size_t maxlength, const char* fmt, va_list ap) {
    size_t len = SafeVsprintf(dest, maxlength, fmt, ap);

    for (size_t i = 0; i < len; i++) {
#if defined(KE_WINDOWS)
        if (dest[i] == '/')
            dest[i] = '\\';
#else
        if (dest[i] == '\\')
            dest[i] = '/';
#endif
    }

    return len;
}

static inline size_t
Format(char* dest, size_t maxlength, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t len = FormatVa(dest, maxlength, fmt, ap);
    va_end(ap);
    return len;
}

} // namespace path
} // namespace ke

#endif // _include_amtl_os_path_h_
