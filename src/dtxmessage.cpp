#include "idevice/dtxmessage.h"

#include <string>
#include <fstream>

using namespace idevice;

static const int kDTXMessagePayloadHeaderSize = 0x10;

static inline void write_buffer_to_file(std::string filename, const char* buffer, uint64_t size)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    file.write(buffer, size);
    file.close();
}

// static
std::unique_ptr<DTXMessage> DTXMessage::Deserialize(const char* bytes, size_t size) {
  /* ONLY FOR DEBUG
  static int count = 0;
  count++;
  std::string filename = std::string("dtxmessage_") + std::to_string(count) + ".bin";
  write_buffer_to_file(filename, bytes, size);
  */
  
  // DTXMessagePayload Memory Layout:
  // |-----------------------------------------------------------|
  // |  0  1  2  3  |  4  5  6  7  |  8  9  A  B  |  C  D  E  F  |
  // |-----------------------------------------------------------|
  // |  msg_type    |  aux_len     |  total_len                  | // `DTXMessagePayloadHeader header`, header of DTXMessagePayload
  // |  auxiliary (size=aux_len)                                 | // `DTXPrimitiveArray auxiliary`, parameters of the function
  // |  ...                                                      |
  // |  payload   (size=total_len - aux_len)                     | // `NSKeyedArchiver selector`, name of the function(or return type)
  // |  ...                                                      |
  // |-----------------------------------------------------------|
  uint32_t message_type = *(uint32_t*)(bytes);
  uint32_t auxiliary_length = *(uint32_t*)(bytes + 0x04);
  uint64_t total_length = *(uint64_t*)(bytes + 0x08);
  uint64_t payload_length = total_length - auxiliary_length;
  // TODO: #if DEBUG
  printf("message_type: %d\n", message_type);
  printf("auxiliary_length: %d\n", auxiliary_length);
  printf("total_length: %llu\n", total_length);
  printf("payload_length: %llu\n", payload_length);
  // TODO: #endif
  
  const char* auxiliary_ptr = bytes + kDTXMessagePayloadHeaderSize;
  const char* payload_ptr = bytes + kDTXMessagePayloadHeaderSize + auxiliary_length;
  
  std::unique_ptr<DTXMessage> message = std::make_unique<DTXMessage>(message_type);
  message->SetAuxiliary(DTXPrimitiveArray::Deserialize(bytes + kDTXMessagePayloadHeaderSize, auxiliary_length));
  message->SetPayloadBuffer(const_cast<char*>(payload_ptr), payload_length, true);
  if (message_type == 7) {
    printf("TODO: DecompressedData(), use zlib\n"); // TODO
  }
  
  return message;
}

void DTXMessage::SetPayloadBuffer(char* buffer, size_t size, bool should_copy) {
  payload_size_ = size;
  
  if (should_copy) {
    payload_ = static_cast<char*>(malloc(size));
    memcpy(payload_, buffer, size);
  } else {
    payload_ = buffer;
  }
}
