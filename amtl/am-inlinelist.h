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

#ifndef _include_amtl_inline_list_h_
#define _include_amtl_inline_list_h_

#include <amtl/am-cxx.h>
#include <assert.h>
#include <stddef.h>

namespace ke {

template <typename T>
class InlineList;

// Objects can recursively inherit from InlineListNode in order to have
// membership in an InlineList<T>.
template <typename T>
class InlineListNode
{
    friend class InlineList<T>;

  public:
    InlineListNode()
     : next_(nullptr),
       prev_(nullptr)
    {}

    InlineListNode(InlineListNode* next, InlineListNode* prev)
     : next_(next),
       prev_(prev)
    {}

    bool isInList() const {
        assert(!!next_ == !!prev_);
        return !!next_;
    }

    void removeFromParentList() {
        prev_->next_ = next_;
        next_->prev_ = prev_;
        next_ = nullptr;
        prev_ = nullptr;
    }

  protected:
    InlineListNode* next_;
    InlineListNode* prev_;
};

// An InlineList is a linked list that threads link pointers through objects,
// rather than allocating node memory. A node can be in at most one list at
// any time.
//
// Since InlineLists are designed to be very cheap, there is no requirement
// that elements be removed from a list once the list is destructed. However,
// for as long as the list is alive, all of its contained nodes must also
// be alive.
template <typename T>
class InlineList
{
    typedef InlineListNode<T> Node;

    Node head_;
    size_t length_;

    // Work around a clang bug where we can't initialize with &head_ in the ctor.
    inline Node* head() {
        return &head_;
    }

  public:
    InlineList()
     : head_(head(), head()),
       length_(0)
    {}

    ~InlineList() {
#if !defined(NDEBUG)
        // Remove all items to clear their next/prev fields.
        while (begin() != end())
            remove(*begin());
#endif
    }

  public:
    class iterator
    {
        friend class InlineList;
        Node* iter_;

      public:
        iterator(Node* iter)
         : iter_(iter)
        {}

        iterator& operator ++() {
            iter_ = iter_->next_;
            return *this;
        }
        iterator operator ++(int) {
            iterator old(*this);
            iter_ = iter_->next_;
            return old;
        }
        T* operator *() {
            return static_cast<T*>(iter_);
        }
        T* operator ->() {
            return static_cast<T*>(iter_);
        }
        bool operator !=(const iterator& where) const {
            return iter_ != where.iter_;
        }
        bool operator ==(const iterator& where) const {
            return iter_ == where.iter_;
        }
    };

    class reverse_iterator
    {
        friend class InlineList;
        Node* iter_;

      public:
        reverse_iterator(Node* iter)
         : iter_(iter)
        {}

        reverse_iterator& operator ++() {
            iter_ = iter_->prev_;
            return *this;
        }
        reverse_iterator operator ++(int) {
            reverse_iterator old(*this);
            iter_ = iter_->prev_;
            return old;
        }
        T* operator *() {
            return static_cast<T*>(iter_);
        }
        T* operator ->() {
            return static_cast<T*>(iter_);
        }
        bool operator !=(const reverse_iterator& where) const {
            return iter_ != where.iter_;
        }
        bool operator ==(const reverse_iterator& where) const {
            return iter_ == where.iter_;
        }
    };

    iterator begin() {
        return iterator(head_.next_);
    }
    reverse_iterator rbegin() {
        return reverse_iterator(head_.prev_);
    }

    iterator end() {
        return iterator(&head_);
    }
    reverse_iterator rend() {
        return reverse_iterator(&head_);
    }

    template <typename IteratorType>
    IteratorType erase(IteratorType& at) {
        IteratorType next = at;
        next++;

        remove(at.iter_);

        // Iterator is no longer valid.
        at.iter_ = nullptr;

        return next;
    }

    bool empty() const {
        return head_.next_ == &head_;
    }

    void remove(Node* t) {
        t->prev_->next_ = t->next_;
        t->next_->prev_ = t->prev_;
        length_--;

#if !defined(NDEBUG)
        t->next_ = nullptr;
        t->prev_ = nullptr;
#endif
    }

    void append(Node* t) {
        assert(!t->next_);
        assert(!t->prev_);

        t->prev_ = head_.prev_;
        t->next_ = &head_;
        head_.prev_->next_ = t;
        head_.prev_ = t;
        length_++;
    }

    size_t length() const {
        return length_;
    }
};

} // namespace ke

#endif // _include_amtl_inline_list_h_
