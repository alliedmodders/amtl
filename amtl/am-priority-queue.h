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

#include <amtl/am-algorithm.h>
#include <amtl/am-cxx.h>
#include <amtl/am-moveable.h>
#include <amtl/am-vector.h>

namespace ke {

// Basic priority queue implemented using a binary heap on top of Vector. The
// "IsHigherPriority" callable must take a left and right value of type T, and
// return whether the left value should be dequeued before the right.
template <typename T, typename IsHigherPriority = LessThan<T>,
          typename AllocPolicy = SystemAllocatorPolicy>
class PriorityQueue final
{
  public:
    explicit PriorityQueue(IsHigherPriority hp = IsHigherPriority(), AllocPolicy ap = AllocPolicy())
     : impl_(ap),
       is_higher_priority_(hp)
    {}

    PriorityQueue(PriorityQueue&& other)
     : impl_(Move(other.impl_)),
       is_higher_priority_(Move(other.is_higher_priority_))
    {}

    template <typename U>
    bool add(U&& item) {
        if (!impl_.append(ke::Forward<U>(item)))
            return false;
        if (impl_.length() > 1)
            propagateUp(impl_.length() - 1);
        return true;
    }

    bool empty() const {
        return impl_.empty();
    }

    const T& peek() const {
        assert(!empty());
        return impl_[0];
    }

    T popCopy() {
        assert(!empty());

        if (impl_.length() == 1)
            return impl_.popCopy();

        T top = Move(impl_[0]);
        impl_[0] = Move(impl_.back());
        impl_.pop();
        propagateDown(0);
        return top;
    }

  private:
    void propagateUp(size_t at) {
        size_t cursor = at;
        T key(ke::Move(impl_[cursor]));
        while (cursor != 0) {
            size_t parent = parentOf(cursor);
            if (!is_higher_priority_(key, impl_[parent]))
                break;

            impl_[cursor] = ke::Move(impl_[parent]);
            cursor = parent;
        }

        impl_[cursor] = ke::Move(key);
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
            ke::Swap(impl_[bestChild], impl_[cursor]);
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
        return leftChildOf(at) < impl_.length();
    }
    bool hasRightChild(size_t at) const {
        return rightChildOf(at) < impl_.length();
    }

  private:
    PriorityQueue(const PriorityQueue& other) = delete;
    void operator =(const PriorityQueue& other) = delete;

  private:
    Vector<T, AllocPolicy> impl_;
    IsHigherPriority is_higher_priority_;
};

} // namespace ke

#endif // _include_amtl_am_priority_queue_h_
