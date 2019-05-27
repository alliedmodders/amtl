// vim: set sts=8 ts=2 sw=2 tw=99 et:
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

#ifndef _include_amtl_am_raii_h_
#define _include_amtl_am_raii_h_

namespace ke {

template <typename T>
class SaveAndSet
{
  public:
    SaveAndSet(T* location, const T& value)
     : location_(location),
       old_(*location)
    {
        *location_ = value;
    }
    ~SaveAndSet() {
        *location_ = old_;
    }

  private:
    T* location_;
    T old_;
};

template <typename T>
class StackLinked
{
  public:
    StackLinked<T>(T** prevp)
     : prevp_(prevp),
       prev_(*prevp)
    {
        *prevp_ = static_cast<T*>(this);
    }
    virtual ~StackLinked() {
        assert(*prevp_ == this);
        *prevp_ = prev_;
    }

  protected:
    T** prevp_;
    T* prev_;
};

template <typename T>
T
ReturnAndVoid(T& t)
{
    T saved = t;
    t = T();
    return saved;
}

} // namespace ke

#endif // _include_amtl_am_raii_h_
