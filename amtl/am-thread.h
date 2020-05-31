// vim: set sts=8 ts=4 sw=4 tw=99 et:
//
// Copyright (C) 2020 David Anderson and AlliedModders LLC
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

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include <amtl/am-platform.h>

#if defined(KE_POSIX)
# include <pthread.h>
#elif defined(KE_WINDOWS)
# include <codecvt>
# include <locale>
# include <string>

# include <Windows.h>
# include <processthreadsapi.h>
# include <sdkddkver.h>
#endif

namespace ke {

namespace impl {

#if defined(KE_WINDOWS)
static void SetThreadName(HANDLE thread, const char* name) {
    auto module = LoadLibraryA("kernel32.dll");
    if (!module)
        return;

    typedef HRESULT (WINAPI * SetThreadDescriptionFn)(HANDLE, PCWSTR);
    auto fn = (SetThreadDescriptionFn)GetProcAddress(module, "SetThreadDescription");
    if (!fn)
        return;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring utf16 = converter.from_bytes(name);
    fn(thread, utf16.c_str());
	
    FreeLibrary(module);
}
#endif

} // namespace detail

static inline void SetThreadName(const char* name) {
#if defined(KE_MACOSX)
    pthread_setname_np(name);
#elif defined(KE_POSIX)
    pthread_setname_np(pthread_self(), name);
#elif defined(KE_WINDOWS)
    impl::SetThreadName(::GetCurrentThread(), name);
#else
    (void)name;
#endif
}

static inline void SetThreadName(std::thread* thread, const char* name) {
#if defined(KE_POSIX) && !defined(KE_MACOSX)
    pthread_setname_np(thread->native_handle(), name);
#elif defined(KE_WINDOWS)
    impl::SetThreadName(thread->native_handle(), name);
#else
    (void)thread;
    (void)name;
#endif
}

// Helper to allocate named threads. We can't subclass std::thread for this
// because the order of construction would be wrong.
template <class Function, class... Args>
static inline std::unique_ptr<std::thread> NewThread(const char* name, Function&& f, Args&&... args)
{
    auto callback = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
    auto fn = [](const std::string& name, decltype(callback)&& callback) -> void {
        SetThreadName(name.c_str());
        callback();
    };
    return std::make_unique<std::thread>(std::move(fn), std::string(name), std::move(callback));
}

} // namespace ke

