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

#include <am-algorithm.h>
#include <assert.h>
#include "runner.h"

using namespace ke;

class ComparableObject
{
  public:
    ComparableObject() :
      m_iValue(0),
      m_fValue(0.0F)
    {
    }
    ComparableObject(const int iValue, const float fValue) :
      m_iValue(iValue),
      m_fValue(fValue)
    {
    }
    bool operator<(const ComparableObject &other) const
    {
      return Compare(other) == -1;
    }
    bool operator>(const ComparableObject &other) const
    {
      return Compare(other) == 1;
    }
  public:
    int m_iValue;
    float m_fValue;
  private:
    int Compare(const ComparableObject &other) const
    {
      if (m_iValue != other.m_iValue)
        return (m_iValue < other.m_iValue ? -1 : 1);
      else if (m_fValue != other.m_fValue)
        return (m_fValue < other.m_fValue ? -1 : 1);
      return 0;
    }
};

#define CHECK_VALUE(arrayVar, index, value) if (!check(arrayVar[index] == value, #arrayVar "[" #index "] = " #value)) return false
#define CHECK_COMPARABLE_OBJECT(arrayVar, index, iValue, fValue) if (!check(arrayVar[index].m_iValue == iValue, #arrayVar "[" #index "].m_iValue = " #iValue)) return false;\
if (!check(arrayVar[index].m_fValue == fValue, #arrayVar "[" #index "].m_fValue = " #fValue)) return false

class TestSort : public Test
{
  public:
    TestSort() :
      Test("Sorting")
    {
    }
  public:
    bool test_ascending()
    {
      int array[10] = { 0, 9999, 0x7fff, 543, 7658765, 43423, 654, 8, 9, 0 };

      Sort(array, 10, LessThen<int>);

      CHECK_VALUE(array, 0, 0);
      CHECK_VALUE(array, 1, 0);
      CHECK_VALUE(array, 2, 8);
      CHECK_VALUE(array, 3, 9);
      CHECK_VALUE(array, 4, 543);
      CHECK_VALUE(array, 5, 654);
      CHECK_VALUE(array, 6, 9999);
      CHECK_VALUE(array, 7, 0x7fff);
      CHECK_VALUE(array, 8, 43423);
      CHECK_VALUE(array, 9, 7658765);

      return true;
    }
    bool test_descending()
    {
      int array[12] = { 1, 0x7fff, 0, 5, 4, 6766, 666, 423, 999, 543, 3212, 55 };

      Sort(array, 12, GreaterThen<int>);

      CHECK_VALUE(array, 0, 0x7fff);
      CHECK_VALUE(array, 1, 6766);
      CHECK_VALUE(array, 2, 3212);
      CHECK_VALUE(array, 3, 999);
      CHECK_VALUE(array, 4, 666);
      CHECK_VALUE(array, 5, 543);
      CHECK_VALUE(array, 6, 423);
      CHECK_VALUE(array, 7, 55);
      CHECK_VALUE(array, 8, 5);
      CHECK_VALUE(array, 9, 4);
      CHECK_VALUE(array, 10, 1);
      CHECK_VALUE(array, 11, 0);

      return true;
    }
    bool test_comparable_object_ascending()
    {
      ComparableObject array[7] = { ComparableObject(), ComparableObject(-9, 99.0F), ComparableObject(-9, 100.0F), ComparableObject(0, -990.0F), ComparableObject(0x7fff, (float)0x7fff), ComparableObject(0x7fff, 0.0F), ComparableObject(-20, (float)0x7fff) };

      Sort(array, 7, LessThen<ComparableObject>);

      CHECK_COMPARABLE_OBJECT(array, 0, -20, (float)0x7fff);
      CHECK_COMPARABLE_OBJECT(array, 1, -9, 99.0F);
      CHECK_COMPARABLE_OBJECT(array, 2, -9, 100.0F);
      CHECK_COMPARABLE_OBJECT(array, 3, 0, -990.0F);
      CHECK_COMPARABLE_OBJECT(array, 4, 0, 0.0F);
      CHECK_COMPARABLE_OBJECT(array, 5, 0x7fff, 0.0F);
      CHECK_COMPARABLE_OBJECT(array, 6, 0x7fff, (float)0x7fff);

      return true;
    }
    bool test_comparable_object_descending()
    {
      ComparableObject array[7] = { ComparableObject(), ComparableObject(-9, 99.0F), ComparableObject(-9, 100.0F), ComparableObject(0, -990.0F), ComparableObject(0x7fff, (float)0x7fff), ComparableObject(0x7fff, 0.0F), ComparableObject(-20, (float)0x7fff) };

      Sort(array, 7, GreaterThen<ComparableObject>);

      CHECK_COMPARABLE_OBJECT(array, 0, 0x7fff, (float)0x7fff);
      CHECK_COMPARABLE_OBJECT(array, 1, 0, 0.0F);
      CHECK_COMPARABLE_OBJECT(array, 2, 0, -990.0F);
      CHECK_COMPARABLE_OBJECT(array, 3, -9, 100.0F);
      CHECK_COMPARABLE_OBJECT(array, 4, -9, 99.0F);
      CHECK_COMPARABLE_OBJECT(array, 5, 0x7fff, 0.0F);
      CHECK_COMPARABLE_OBJECT(array, 6, -20, (float)0x7fff);

      return true;
    }
    bool test_using_custom_predicate()
    {
      auto predicate = [](const int value1, const int value2) -> bool
      {
        return (value1 < 50) && (value1 < value2);
      };

      int array[16] = { 0, 654, 432, 45, 3, 57, 10, 53, 99, 100, 5, 4, 1, 2, 5438, 543 };

      Sort(array, 16, predicate);

      CHECK_VALUE(array, 0, 0);
      CHECK_VALUE(array, 1, 1);
      CHECK_VALUE(array, 2, 2);
      CHECK_VALUE(array, 3, 3);
      CHECK_VALUE(array, 4, 4);
      CHECK_VALUE(array, 5, 5);
      CHECK_VALUE(array, 6, 10);
      CHECK_VALUE(array, 7, 45);
      CHECK_VALUE(array, 8, 543);
      CHECK_VALUE(array, 9, 5438);
      CHECK_VALUE(array, 10, 100);
      CHECK_VALUE(array, 11, 99);
      CHECK_VALUE(array, 12, 53);
      CHECK_VALUE(array, 13, 57);
      CHECK_VALUE(array, 14, 432);
      CHECK_VALUE(array, 15, 654);

      return true;
    }
    bool Run() override
    {
      if (!test_ascending())
        return false;
      else if (!test_descending())
        return false;
      else if (!test_comparable_object_ascending())
        return false;
      else if (!test_comparable_object_descending())
        return false;
      else if (!test_using_custom_predicate())
        return false;

      return true;
    }
} sTestSort;

