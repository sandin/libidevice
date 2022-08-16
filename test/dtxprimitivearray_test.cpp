#include "idevice/dtxprimitivearray.h"

#include <map>
#include <cstdlib> // abs
#include <gtest/gtest.h>

using namespace idevice;

TEST(DTXPrimitiveArrayTest, Create) {
  DTXPrimitiveArray array;
  
  // 0: kNull
  array.Append(DTXPrimitiveValue());
  
  // 1: kString
  const char* str = "hello world";
  array.Append(DTXPrimitiveValue(str));
               
  // 2: kBuffer
  char buffer[] = { 0x01, 0x02, 0x03, 0x04 };
  array.Append(DTXPrimitiveValue(buffer, sizeof(buffer)));
               
  // 3: kSignedInt32
  array.Append(DTXPrimitiveValue((int32_t)32));
  
  // 4: kSignedInt64
  array.Append(DTXPrimitiveValue((int64_t)64));
  
  // 5: kFloat32
  array.Append(DTXPrimitiveValue((float)3.2));
  
  // 6: kFloat64
  array.Append(DTXPrimitiveValue((double)6.4));
  
  // 7: kInteger
  array.Append(DTXPrimitiveValue((uint64_t)64));
  
  
  ASSERT_EQ(8, array.Size());
  
  DTXPrimitiveValue& value0 = array.At(0);
  ASSERT_EQ(DTXPrimitiveValue::kNull, value0.GetType());
  ASSERT_EQ(0, value0.Size());
  
  DTXPrimitiveValue& value1 = array.At(1);
  ASSERT_EQ(DTXPrimitiveValue::kString, value1.GetType());
  ASSERT_EQ(strlen(str) + 1 /* \0 */, value1.Size());
  ASSERT_STREQ(str, value1.ToStr());
  
  DTXPrimitiveValue& value2 = array.At(2);
  ASSERT_EQ(DTXPrimitiveValue::kBuffer, value2.GetType());
  ASSERT_EQ(4, value2.Size());
  ASSERT_EQ(buffer[0], value2.ToBuffer()[0]);
  ASSERT_EQ(buffer[1], value2.ToBuffer()[1]);
  ASSERT_EQ(buffer[2], value2.ToBuffer()[2]);
  ASSERT_EQ(buffer[3], value2.ToBuffer()[3]);
  
  DTXPrimitiveValue& value3 = array.At(3);
  ASSERT_EQ(DTXPrimitiveValue::kSignedInt32, value3.GetType());
  ASSERT_EQ(4, value3.Size());
  ASSERT_EQ(32, value3.ToSignedInt32());
  
  DTXPrimitiveValue& value4 = array.At(4);
  ASSERT_EQ(DTXPrimitiveValue::kSignedInt64, value4.GetType());
  ASSERT_EQ(8, value4.Size());
  ASSERT_EQ(64, value4.ToSignedInt64());
  
  DTXPrimitiveValue& value5 = array.At(5);
  ASSERT_EQ(DTXPrimitiveValue::kFloat32, value5.GetType());
  ASSERT_EQ(4, value5.Size());
  ASSERT_FLOAT_EQ(3.2, value5.ToFloat32());
  
  DTXPrimitiveValue& value6 = array.At(6);
  ASSERT_EQ(DTXPrimitiveValue::kFloat64, value6.GetType());
  ASSERT_EQ(8, value6.Size());
  ASSERT_DOUBLE_EQ(6.4, value6.ToFloat64());
  
  DTXPrimitiveValue& value7 = array.At(7);
  ASSERT_EQ(DTXPrimitiveValue::kInteger, value7.GetType());
  ASSERT_EQ(8, value7.Size());
  ASSERT_EQ(64, value7.ToInteger());
}
