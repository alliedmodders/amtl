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
#include "runner.h"
#include <am-platform.h>
#include <gtest/gtest.h>
#include <os/am-shared-library.h>
#include <os/am-path.h>

using namespace ke;

TEST(System, SharedLibs)
{
  const char* libname = "unknown-library";
#if defined(KE_WINDOWS)
  const char* symbol = "GetProcAddress";
  libname = "kernel32.dll";
#elif defined(KE_POSIX)
  const char* symbol = "malloc";
# if defined(KE_MACOSX)
  libname = "libc.dylib";
# elif defined(KE_LINUX)
  libname = "libc.so.6";
# endif
#endif

  RefPtr<SharedLib> lib = SharedLib::Open(libname, nullptr, 0);
  EXPECT_TRUE(lib);
  EXPECT_NE(lib->lookup(symbol), nullptr);
  EXPECT_NE(lib->get<void*>(symbol), nullptr);
}

TEST(System, Paths)
{
#if defined(KE_WINDOWS)
  const char* bad = "C:/egg.txt";
  const char* good = "C:\\egg.txt";
#else
  const char* bad = "\\egg.txt";
  const char* good = "/egg.txt";
#endif

  char buffer[255];
  path::Format(buffer, sizeof(buffer), "%s", bad);
  EXPECT_EQ(strcmp(buffer, good), 0);
}
