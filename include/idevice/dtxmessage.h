#ifndef IDEVICE_DTXMESSAGE_H
#define IDEVICE_DTXMESSAGE_H

#include <memory> // std::unique_ptr

#include "idevice/idevice.h"
#include "idevice/dtxprimitivearray.h"

namespace idevice {

struct DTXMessageHeader {
  uint32_t magic;                 // +0x00, len=4
  uint32_t message_header_size;   // +0x04, len=4
  uint16_t fragment_id;           // +0x08, len=2
  uint16_t fragment_count;        // +0x0a, len=2
  uint32_t length;                // +0x0c, len=4
  uint32_t identifier;            // +0x10, len=4
  uint32_t conversation_index;    // +0x14, len=4
  uint32_t channel_code;          // +0x18, len=4
  uint32_t expects_reply;         // +0x1c, len=4
};

class DTXMessage {
 public:
  DTXMessage(uint32_t message_type) : message_type_(message_type) {}
  virtual ~DTXMessage() {}
  
  static std::unique_ptr<DTXMessage> Deserialize(const char* bytes, size_t size);
  
  void SetPayloadBuffer(char* buffer, size_t size, bool should_copy);
  void SetAuxiliary(std::unique_ptr<DTXPrimitiveArray>&& auxiliary) { auxiliary_ = std::move(auxiliary); }
  
 private:
  char* payload_ = nullptr;
  size_t payload_size_ = 0;
  uint32_t message_type_ = 0;
  std::unique_ptr<DTXPrimitiveArray> auxiliary_;
}; // class DTXMessage

}  // namespace idevice

#endif // IDEVICE_DTXMESSAGE_H
