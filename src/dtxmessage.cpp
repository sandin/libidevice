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
std::unique_ptr<DTXMessage> DTXMessage::Create(const char* bytes, size_t size) {
  static int count = 0;
  count++;
  std::string filename = std::string("dtxmessage_") + std::to_string(count) + ".bin";
  write_buffer_to_file(filename, bytes, size);

  uint32_t message_type = *(uint32_t*)(bytes);
  uint32_t auxiliary_length = *(uint32_t*)(bytes + 0x04);
  uint64_t total_length = *(uint64_t*)(bytes + 0x08);
  uint64_t payload_length = total_length - auxiliary_length;
  printf("message_type: %d\n", message_type);
  printf("auxiliary_length: %d\n", auxiliary_length);
  printf("total_length: %llu\n", total_length);
  printf("payload_length: %llu\n", payload_length);
  
  const char* auxiliary_ptr = bytes + kDTXMessagePayloadHeaderSize;
  const char* payload_ptr = bytes + kDTXMessagePayloadHeaderSize + auxiliary_length;
  printf("auxiliary:\n");
  hexdump((void*)auxiliary_ptr, (int)auxiliary_length, 0); // FIXME: debug only
  printf("payload:\n");
  hexdump((void*)payload_ptr, (int)payload_length, 0); // FIXME: debug only
  
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
