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
#ifndef _include_amtl_os_sharedlib_h_
#define _include_amtl_os_sharedlib_h_

#include <amtl/am-platform.h>
#include <amtl/am-refcounting.h>
#if defined(KE_WINDOWS)
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    include <amtl/os/am-system-errors.h>
#else
#    include <amtl/am-string.h>
#    include <dlfcn.h>
#endif

namespace ke {

class SharedLib : public Refcounted<SharedLib>
{
#if defined(KE_WINDOWS)
    typedef HMODULE SysType;
#else
    typedef void* SysType;
#endif

  public:
    SharedLib()
     : lib_(nullptr) {
    }
    explicit SharedLib(const SysType& lib)
     : lib_(lib) {
    }
    explicit SharedLib(const char* path) {
#if defined(KE_WINDOWS)
        lib_ = LoadLibraryA(path);
#else
        lib_ = dlopen(path, RTLD_NOW);
#endif
    }
    ~SharedLib() {
        if (lib_ == nullptr)
            return;

#if defined(KE_WINDOWS)
        FreeLibrary(lib_);
#else
        dlclose(lib_);
#endif
    }

    explicit operator bool() const {
        return !!lib_;
    }

    static inline AlreadyRefed<SharedLib> Open(const char* path, char* error = nullptr,
                                               size_t maxlength = 0) {
        RefPtr<SharedLib> lib = new SharedLib(path);
        if (!lib || !lib->valid()) {
            if (!error || !maxlength)
                return nullptr;
#if defined(KE_WINDOWS)
            FormatSystemError(error, maxlength);
#else
            SafeStrcpy(error, maxlength, dlerror());
#endif
            return nullptr;
        }
        return lib.forget();
    }

    void* lookup(const char* symbol) {
#if defined(KE_WINDOWS)
        return reinterpret_cast<void*>(GetProcAddress(lib_, symbol));
#else
        return dlsym(lib_, symbol);
#endif
    }

    template <typename T>
    T get(const char* symbol) {
        return reinterpret_cast<T>(lookup(symbol));
    }

    bool valid() const {
        return !!lib_;
    }

  private:
    SysType lib_;
};

} // namespace ke

#endif // _include_amtl_os_sharedlib_h_
