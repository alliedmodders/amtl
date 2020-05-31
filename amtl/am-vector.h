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

#pragma once

#include <stddef.h>

#include <vector>

namespace ke {

template <typename T>
static inline void MoveExtend(std::vector<T>* dest, std::vector<T>* src) {
    if (dest->empty()) {
        *dest = std::move(*src);
    } else {
        for (size_t i = 0; i < src->size(); i++)
            dest->emplace_back(std::move(src->at(i)));
        src->clear();
    }
}

template <typename T>
static inline T PopBack(std::vector<T>* src) {
    T t = std::move(src->back());
    src->pop_back();
    return t;
}

template <typename T>
static inline void RemoveAt(std::vector<T>* vec, size_t at) {
    vec->erase(vec->begin() + at);
}

template <typename T>
static inline void InsertAt(std::vector<T>* vec, size_t at, const T& item) {
    vec->insert(vec->begin() + at, item);
}

template <typename T>
static inline void InsertAt(std::vector<T>* vec, size_t at, T&& item) {
    vec->insert(vec->begin() + at, std::forward<T>(item));
}

template <typename T, typename... Args>
static inline void EmplaceAt(std::vector<T>* vec, size_t at, Args&&... item) {
    vec->emplace(vec->begin() + at, std::forward<Args>(item)...);
}

} // namespace ke
