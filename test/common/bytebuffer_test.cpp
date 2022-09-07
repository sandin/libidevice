#include "idevice/utils/bytebuffer.h"

#include <gtest/gtest.h>

#include <map>

using namespace idevice;

TEST(ByteBufferTest, Append) {
  ByteBuffer buffer(4096);
  ASSERT_EQ(0, buffer.Size());

  const char data1[] = {0x00, 0x01, 0x02, 0x03};
  buffer.Append(data1, sizeof(data1));
  ASSERT_EQ(4, buffer.Size());

  const void* buffer_read = buffer.GetReadOnlyBuffer(1);
  ASSERT_EQ(0x01, *((char*)buffer_read + 0));
  ASSERT_EQ(0x02, *((char*)buffer_read + 1));
  ASSERT_EQ(0x03, *((char*)buffer_read + 2));

  std::map<uint64_t, ByteBuffer> buffer_map;
  buffer_map.insert(
      std::make_pair(1, std::move(buffer)));  // DO NOT USE `buffer` var after this line

  const char data2[] = {0x04, 0x05, 0x06, 0x07};
  ByteBuffer& buffer1 = buffer_map.at(1);
  printf("buffer1.size=%zu\n", buffer1.Size());
  buffer1.Append(data2, sizeof(data2));
  ASSERT_EQ(8, buffer1.Size());

  const void* buffer_read1 = buffer1.GetReadOnlyBuffer(4);
  ASSERT_EQ(0x04, *((char*)buffer_read1 + 0));
  ASSERT_EQ(0x05, *((char*)buffer_read1 + 1));
  ASSERT_EQ(0x06, *((char*)buffer_read1 + 2));
  ASSERT_EQ(0x07, *((char*)buffer_read1 + 3));
}
