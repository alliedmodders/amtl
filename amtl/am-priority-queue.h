// vim: set sts=8 ts=2 sw=2 tw=99 et:
//
// Copyright (C) 2013-2016, David Anderson and AlliedModders LLC
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

#ifndef _include_amtl_am_priority_queue_h_
#define _include_amtl_am_priority_queue_h_

#include <assert.h>

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include <amtl/am-cxx.h>

namespace ke {

// Basic priority queue implemented using a binary heap on top of vector. The
// "IsHigherPriority" callable must take a left and right value of type T, and
// return whether the left value should be dequeued before the right.
//
// This allows move-and-pop which the STL container does not.
template <typename T, typename IsHigherPriority = std::less<T>>
class PriorityQueue final
{
  public:
    explicit PriorityQueue(IsHigherPriority hp = IsHigherPriority())
       : is_higher_priority_(hp)
    {}

    PriorityQueue(PriorityQueue&& other)
     : impl_(std::move(other.impl_)),
       is_higher_priority_(std::move(other.is_higher_priority_))
    {}

    template <typename U>
    void add(U&& item) {
        impl_.push_back(std::forward<U>(item));
        if (impl_.size() > 1)
            propagateUp(impl_.size() - 1);
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        impl_.emplace_back(std::forward<Args>(args)...);
        if (impl_.size() > 1)
            propagateUp(impl_.size() - 1);
    }

    bool empty() const {
        return impl_.empty();
    }

    const T& peek() const {
        assert(!empty());
        return impl_[0];
    }

    T pop() {
        assert(!empty());

        T top = std::move(impl_[0]);
        if (impl_.size() == 1) {
            impl_.pop_back();
        } else {
            impl_[0] = std::move(impl_.back());
            impl_.pop_back();
            propagateDown(0);
        }
        return top;
    }

  private:
    void propagateUp(size_t at) {
        size_t cursor = at;
        T key(std::move(impl_[cursor]));
        while (cursor != 0) {
            size_t parent = parentOf(cursor);
            if (!is_higher_priority_(key, impl_[parent]))
                break;

            impl_[cursor] = std::move(impl_[parent]);
            cursor = parent;
        }

        impl_[cursor] = std::move(key);
    }

    void propagateDown(size_t at) {
        size_t cursor = at;
        while (hasLeftChild(cursor)) {
            // Pick the child with the highest priority.
            size_t bestChild = leftChildOf(cursor);
            if (hasRightChild(cursor)) {
                size_t rightIndex = rightChildOf(cursor);
                if (is_higher_priority_(impl_[rightIndex], impl_[bestChild]))
                    bestChild = rightIndex;
            }

            // If the highest priority child is not higher priority than the cursor,
            // then we are finished.
            if (!is_higher_priority_(impl_[bestChild], impl_[cursor]))
                break;

            // Otherwise, swap and move on.
            std::swap(impl_[bestChild], impl_[cursor]);
            cursor = bestChild;
        }
    }

    static constexpr size_t parentOf(size_t at) {
#if __cplusplus >= 201402L
        assert(at > 0);
#endif
        return (at - 1) / 2;
    }
    static constexpr size_t leftChildOf(size_t at) {
        return (at * 2) + 1;
    }
    static constexpr size_t rightChildOf(size_t at) {
        return (at * 2) + 2;
    }

    bool hasLeftChild(size_t at) const {
        return leftChildOf(at) < impl_.size();
    }
    bool hasRightChild(size_t at) const {
        return rightChildOf(at) < impl_.size();
    }

  private:
    PriorityQueue(const PriorityQueue& other) = delete;
    void operator =(const PriorityQueue& other) = delete;

  private:
    std::vector<T> impl_;
    IsHigherPriority is_higher_priority_;
};

} // namespace ke

#endif // _include_amtl_am_priority_queue_h_
