#include "idevice/dtxmessage.h"

using namespace idevice;


// static
std::unique_ptr<DTXMessage> DTXMessage::Create(const char* bytes, size_t size) {
  hexdump((void*)bytes, (int)size, 0); // FIXME: debug only

  std::unique_ptr<DTXMessage> message = std::make_unique<DTXMessage>();
  
  uint32_t message_type = *(uint32_t*)(bytes);
  uint32_t auxiliary_length = *(uint32_t*)(bytes + 0x04);
  uint64_t total_length = *(uint64_t*)(bytes + 0x08);
  const char* payload_ptr = bytes + 0x10;
  uint64_t payload_length = total_length - auxiliary_length;
  
  printf("message_type: %ul\n", message_type);
  printf("auxiliary_length: %ul\n", auxiliary_length);
  printf("total_length: %llu\n", total_length);
  printf("payload_length: %llu\n", payload_length);
  
  return message;
}
