#include "idevice/dtxmessagetransmitter.h"

#include <gtest/gtest.h>

#include <cstdlib>  // abs
#include <fstream>
#include <map>
#include <memory>  // std::shared_ptr
#include <vector>

#include "idevice/bytebuffer.h"
#include "idevice/dtxmessage.h"
#include "idevice/idevice.h"

#ifdef ENABLE_NSKEYEDARCHIVE_TEST
#include "nskeyedarchiver/kamap.hpp"
#include "nskeyedarchiver/nskeyedarchiver.hpp"
#include "nskeyedarchiver/nskeyedunarchiver.hpp"
#endif

using namespace idevice;

static inline void write_buffer_to_file(std::string filename, const char* buffer, uint64_t size) {
  std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
  file.write(buffer, size);
  file.close();
}

#define ASSERT_BYTES_EQ(b1, b2, sz)                                                         \
  {                                                                                         \
    for (int i = 0; i < sz; ++i) {                                                          \
      if ((char)*(b1 + i) != (char)*(b2 + i)) {                                             \
        printf("two bytes are not equals at pos: 0x%x, c1=%X, c2=%X\n", i, (char)*(b1 + i), \
               (char)*(b2 + i));                                                            \
        ASSERT_EQ((char)*(b1 + i), (char)*(b2 + i));                                        \
      }                                                                                     \
    }                                                                                       \
  }

#ifdef ENABLE_NSKEYEDARCHIVE_TEST
TEST(DTXMessageTransmitterTest, TransmitMessage_SingleFragment) {
  std::shared_ptr<DTXMessage> message =
      DTXMessage::CreateWithSelector("_requestChannelWithCode:identifier:");
  uint32_t msg_identifier = 1;
  message->SetIdentifier(msg_identifier);

  // aux_0: type=3, key=code, value=1
  uint32_t channel_code = 2;
  message->AppendAuxiliary(DTXPrimitiveValue(static_cast<int32_t>(channel_code)));

  // aux_1: type=2, key=identifier, value=bplist(str)
  const char* channel_identifier = "com.apple.instruments.server.services.deviceinfo";
  message->AppendAuxiliary(nskeyedarchiver::KAValue(channel_identifier));

  ByteBuffer send_buffer(8192);
  DTXMessageTransmitter transmitter;
  transmitter.TransmitMessage(message, {msg_identifier, 0, 0},
                              [&](const char* data, size_t size) -> bool {
                                send_buffer.Append(data, size);
                                return true;
                              });
  idevice::hexdump(send_buffer.GetBuffer(0), send_buffer.Size(), 0);
  printf("=================================\n");

  // clang-format off
  const unsigned char expect_data[451] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
  //  <DTXMessageHeader>
      0x79, 0x5B, 0x3D, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xA3, 0x01, 0x00, 0x00,
  //  | magic=0x1F3D5B79      | msg_header_size=32    | fg_idx=0  | fg_cnt=1 | length=419          |
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  //  | identifier=1          | conv_idx=0            | channel_code=0        | expects_reply=1    |
  //  <DTXMessagePayloadHeader>
      0x02, 0x00, 0x00, 0x00, 0xE4, 0x00, 0x00, 0x00, 0x93, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | msg_type=2            | aux_len=228           | total_len=403                              |
  //  <DTXPrimitiveArray>
      0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | capacity=496                                        | size=212                             |
      0x0A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
  //  | type=10               | type=3                | aux_0:channel_code=2  | type=10            |
      0x02, 0x00, 0x00, 0x00, 0xBC, 0x00, 0x00, 0x00, 0x62, 0x70, 0x6C, 0x69, 0x73, 0x74, 0x30, 0x30,
  //  | type=2                | value_len=188         | aux_1:offset=0x58, value=bplist...
      0xD4, 0x01, 0x03, 0x05, 0x09, 0x02, 0x04, 0x06, 0x0A, 0x58, 0x24, 0x76, 0x65, 0x72, 0x73, 0x69,
  //  bplist:"com.apple.instruments.server.services.deviceinfo"
      0x6F, 0x6E, 0x12, 0x00, 0x01, 0x86, 0xA0, 0x59, 0x24, 0x61, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65,
      0x72, 0x5F, 0x10, 0x0F, 0x4E, 0x53, 0x4B, 0x65, 0x79, 0x65, 0x64, 0x41, 0x72, 0x63, 0x68, 0x69,
      0x76, 0x65, 0x72, 0x54, 0x24, 0x74, 0x6F, 0x70, 0xD1, 0x07, 0x08, 0x54, 0x72, 0x6F, 0x6F, 0x74,
      0x80, 0x01, 0x58, 0x24, 0x6F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x73, 0xA2, 0x0B, 0x0C, 0x55, 0x24,
      0x6E, 0x75, 0x6C, 0x6C, 0x5F, 0x10, 0x30, 0x63, 0x6F, 0x6D, 0x2E, 0x61, 0x70, 0x70, 0x6C, 0x65,
      0x2E, 0x69, 0x6E, 0x73, 0x74, 0x72, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x73, 0x2E, 0x73, 0x65, 0x72,
      0x76, 0x65, 0x72, 0x2E, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x73, 0x2E, 0x64, 0x65, 0x76,
      0x69, 0x63, 0x65, 0x69, 0x6E, 0x66, 0x6F, 0x08, 0x11, 0x1A, 0x1F, 0x29, 0x3B, 0x40, 0x43, 0x48,
      0x4A, 0x53, 0x56, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x8F, 0x62, 0x70, 0x6C, 0x69, 0x73, 0x74, 0x30, 0x30, 0xD4, 0x01, 0x03, 0x05,
  //                          <DTXMessagePayload>
  //                          | payload, offset=0x114, size=175, value=bplist...
      0x09, 0x02, 0x04, 0x06, 0x0A, 0x58, 0x24, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x12, 0x00,
  //  bplist:"_requestChannelWithCode:identifier:"
      0x01, 0x86, 0xA0, 0x59, 0x24, 0x61, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65, 0x72, 0x5F, 0x10, 0x0F,
      0x4E, 0x53, 0x4B, 0x65, 0x79, 0x65, 0x64, 0x41, 0x72, 0x63, 0x68, 0x69, 0x76, 0x65, 0x72, 0x54,
      0x24, 0x74, 0x6F, 0x70, 0xD1, 0x07, 0x08, 0x54, 0x72, 0x6F, 0x6F, 0x74, 0x80, 0x01, 0x58, 0x24,
      0x6F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x73, 0xA2, 0x0B, 0x0C, 0x55, 0x24, 0x6E, 0x75, 0x6C, 0x6C,
      0x5F, 0x10, 0x23, 0x5F, 0x72, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x43, 0x68, 0x61, 0x6E, 0x6E,
      0x65, 0x6C, 0x57, 0x69, 0x74, 0x68, 0x43, 0x6F, 0x64, 0x65, 0x3A, 0x69, 0x64, 0x65, 0x6E, 0x74,
      0x69, 0x66, 0x69, 0x65, 0x72, 0x3A, 0x08, 0x11, 0x1A, 0x1F, 0x29, 0x3B, 0x40, 0x43, 0x48, 0x4A,
      0x53, 0x56, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x82
  };
  // clang-format on
  idevice::hexdump((void*)&expect_data, sizeof(expect_data), 0);

  ASSERT_EQ(send_buffer.Size(), sizeof(expect_data));
  char* actual_ptr = reinterpret_cast<char*>(send_buffer.GetBuffer(0));
  ASSERT_BYTES_EQ(actual_ptr, expect_data, send_buffer.Size());
}

TEST(DTXMessageTransmitterTest, TransmitMessage_MultipleFragments) {
  constexpr size_t kDTXMessageHeaderSize = 0x20;
  constexpr size_t kDTXMessagePayloadHeaderSize = 0x10;

  DTXMessageTransmitter transmitter;
  size_t fragment_payload_size = transmitter.SuggestedFragmentSize() - kDTXMessageHeaderSize;
  size_t total_size = fragment_payload_size * 3.5;

  char* buffer = static_cast<char*>(malloc(total_size));
  char start_char = '0';
  char end_char = '9';
  char cur_char = start_char;
  for (int i = 0; i < total_size; ++i) {
    if (cur_char > end_char) {
      cur_char = start_char;
    }
    buffer[i] = cur_char;
    cur_char++;
  }

  std::shared_ptr<DTXMessage> message = DTXMessage::CreateWithBuffer(buffer, total_size, true);
  free(buffer);
  uint32_t msg_identifier = 1;
  message->SetIdentifier(msg_identifier);

  ByteBuffer send_buffer(8192);
  transmitter.TransmitMessage(message, {msg_identifier, 0, 0, 0},
                              [&](const char* data, size_t size) -> bool {
                                send_buffer.Append(data, size);
                                return true;
                              });
  // idevice::hexdump(send_buffer.GetBuffer(0), send_buffer.Size(), 0);
  write_buffer_to_file("TransmitMessage_MultipleFragments.bin",
                       reinterpret_cast<const char*>(send_buffer.GetBuffer(0)), send_buffer.Size());
  printf("=================================\n");

  ASSERT_EQ(send_buffer.Size(),
            kDTXMessageHeaderSize  // the first fragment of the DTXMessage only contains the header
                + kDTXMessageHeaderSize + kDTXMessagePayloadHeaderSize +
                fragment_payload_size                                  // the #1 fragment
                + kDTXMessageHeaderSize + fragment_payload_size        // the #2 fragment
                + kDTXMessageHeaderSize + fragment_payload_size        // the #3 fragment
                + kDTXMessageHeaderSize + (fragment_payload_size / 2)  // the last fragment
  );
  char* actual_ptr = reinterpret_cast<char*>(send_buffer.GetBuffer(0));

  // clang-format off
  const unsigned char expect_fragment_0_header[] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
  //  <DTXMessageHeader>
      0x79, 0x5B, 0x3D, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0xA0, 0x7F, 0x03, 0x00,
  //  | magic=0x1F3D5B79      | msg_header_size=32    | fg_idx=0  | fg_cnt=4 | length=229280       |
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | identifier=1          | conv_idx=0            | channel_code=0        | expects_reply=0    |
  };
  ASSERT_BYTES_EQ(actual_ptr, expect_fragment_0_header, sizeof(expect_fragment_0_header));
  // length 229280 = 65504 + 65504 + 65504 + 32768
  
  const unsigned char expect_fragment_1_header[] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
  //  <DTXMessageHeader>
      0x79, 0x5B, 0x3D, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0xE0, 0xFF, 0x00, 0x00,
  //  | magic=0x1F3D5B79      | msg_header_size=32    | fg_idx=1  | fg_cnt=4 | length=65504        |
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | identifier=1          | conv_idx=0            | channel_code=0        | expects_reply=0    |
  };
  ASSERT_BYTES_EQ(actual_ptr + 0x20, expect_fragment_1_header, sizeof(expect_fragment_1_header));
  
  const unsigned char expect_fragment_2_header[] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
  //  <DTXMessageHeader>
      0x79, 0x5B, 0x3D, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0xE0, 0xFF, 0x00, 0x00,
  //  | magic=0x1F3D5B79      | msg_header_size=32    | fg_idx=2  | fg_cnt=4 | length=65504        |
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | identifier=1          | conv_idx=0            | channel_code=0        | expects_reply=0    |
  };
  ASSERT_BYTES_EQ(actual_ptr + 0x10020, expect_fragment_2_header, sizeof(expect_fragment_2_header));
  
  const unsigned char expect_fragment_3_header[] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
  //  <DTXMessageHeader>
      0x79, 0x5B, 0x3D, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0xE0, 0xFF, 0x00, 0x00,
  //  | magic=0x1F3D5B79      | msg_header_size=32    | fg_idx=3  | fg_cnt=4 | length=65504        |
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | identifier=1          | conv_idx=0            | channel_code=0        | expects_reply=0    |
  };
  ASSERT_BYTES_EQ(actual_ptr + 0x20020, expect_fragment_3_header, sizeof(expect_fragment_3_header));
  
  const unsigned char expect_fragment_4_header[] = {
  //     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
  //  <DTXMessageHeader>
      0x79, 0x5B, 0x3D, 0x1F, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00,
  //  | magic=0x1F3D5B79      | msg_header_size=32    | fg_idx=4  | fg_cnt=4 | length=32768        |
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  //  | identifier=1          | conv_idx=0            | channel_code=0        | expects_reply=0    |
  };
  ASSERT_BYTES_EQ(actual_ptr + 0x30020, expect_fragment_4_header, sizeof(expect_fragment_4_header));
  // clang-format on
}

#endif  // ENABLE_NSKEYEDARCHIVE_TEST
