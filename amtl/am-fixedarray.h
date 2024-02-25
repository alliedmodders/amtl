// vim: set sts=8 ts=4 sw=4 tw=99 et:
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
#ifndef _include_amtl_fixedarray_h_
#define _include_amtl_fixedarray_h_

#include <assert.h>

#include <vector>

#include <amtl/am-allocator-policies.h>

namespace ke {

template <typename T, typename AllocPolicy = SystemAllocatorPolicy>
class FixedArray : private AllocPolicy
{
  public:
    FixedArray() : size_(0), data_(nullptr)
    {}
    FixedArray(size_t size, AllocPolicy = AllocPolicy()) {
        if (!allocate(size))
            return;
        for (size_t i = 0; i < size_; i++)
            new (&data_[i]) T();
    }
    FixedArray(const FixedArray& other) {
        if (!allocate(other.size()))
            return;
        for (size_t i = 0; i < size_; i++)
            new (&data_[i]) T(other[i]);
    }

    FixedArray(FixedArray&& other) {
        size_ = other.size_;
        data_ = other.data_;
        other.size_ = 0;
        other.data_ = nullptr;
    }

    template <typename U = std::allocator<T>>
    explicit FixedArray(const std::vector<T, U>& other) {
        if (!allocate(other.size()))
            return;

        for (size_t i = 0; i < size_; i++)
            new (&data_[i]) T(other[i]);
    }

    template <typename U = std::allocator<T>>
    explicit FixedArray(std::vector<T, U>&& other) {
        if (!allocate(other.size()))
            return;

        for (size_t i = 0; i < size_; i++)
            new (&data_[i]) T(std::move(other[i]));
    }

    ~FixedArray() {
        destruct();
        deallocate();
    }

    // This call may be skipped if the allocator policy is infallible.
    bool initialize() {
        return size_ == 0 || !!data_;
    }

    size_t size() const {
        return size_;
    }
    T& operator [](size_t index) {
        return at(index);
    }
    const T& operator [](size_t index) const {
        return at(index);
    }
    T& at(size_t index) {
        assert(index < size());
        return data_[index];
    }
    const T& at(size_t index) const {
        assert(index < size());
        return data_[index];
    }
    T& back() {
        assert(size() > 0);
        return data_[size() - 1];
    }
    const T& back() const {
        assert(size() > 0);
        return data_[size() - 1];
    }
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
    bool empty() const { return size_ == 0; }
    T* buffer() const { return data_; }

    FixedArray& operator =(const FixedArray& other) {
        destruct();
        if (size_ != other.size()) {
            deallocate();
            if (!allocate(other.size()))
                return *this;
        }
        for (size_t i = 0; i < size_; i++)
            new (&data_[i]) T(other[i]);
        return *this;
    }
    FixedArray& operator =(FixedArray&& other) {
        size_ = other.size_;
        data_ = other.data_;
        other.size_ = 0;
        other.data_ = nullptr;
        return *this;
    }

    bool operator ==(const FixedArray& other) const {
        if (other.size() != size())
            return false;
        for (size_t i = 0; i < other.size(); i++) {
            if (at(i) != other.at(i))
                return false;
        }
        return true;
    }
    bool operator !=(const FixedArray& other) const {
        return !(*this == other);
    }

  private:
    bool allocate(size_t size) {
        size_ = size;
        data_ = (T*)this->am_malloc(sizeof(T) * size_);
        return !!data_;
    }
    void destruct() {
        for (size_t i = 0; i < size_; i++)
            data_[i].~T();
    }
    void deallocate() {
        this->am_free(data_);
    }

  private:
    size_t size_;
    T* data_;
};

} // namespace ke

#endif // _include_amtl_fixedarray_h_
