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

#ifndef _include_amtl_runner_h_
#define _include_amtl_runner_h_

#include <amtl/am-cxx.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace ke {

class FallibleMalloc
{
  public:
    FallibleMalloc()
     : shouldOutOfMemory_(false),
       ooms_(0),
       overflows_(0)
    {}

    void* am_malloc(size_t amount) {
        if (shouldOutOfMemory_) {
            reportOutOfMemory();
            return nullptr;
        }
#if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
        return malloc(amount);
#else
        return ::malloc(amount);
#endif
    }
    void am_free(void* p) {
#if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
        return free(p);
#else
        return ::free(p);
#endif
    }
    void reportOutOfMemory() {
        ooms_++;
    }
    void reportAllocationOverflow() {
        overflows_++;
    }

    void setOutOfMemory(bool oom) {
        shouldOutOfMemory_ = oom;
    }
    size_t ooms() const {
        return ooms_;
    }
    size_t overflows() const {
        return overflows_;
    }

  private:
    bool shouldOutOfMemory_;
    size_t ooms_;
    size_t overflows_;
};

} // namespace ke

#endif // _include_amtl_runner_h_
