#ifndef IDEVICE_DTXMESSAGE_TRANSMITTER_H
#define IDEVICE_DTXMESSAGE_TRANSMITTER_H

#include "idevice/idevice.h"
#include "idevice/dtxmessage.h"

namespace idevice {

class DTXMessageTransmitter {
 public:
  using Transmitter = std::function<bool(const char*, size_t)>;
  
  DTXMessageTransmitter() {}
  virtual ~DTXMessageTransmitter() {}
  
  bool TransmitMessage(const std::shared_ptr<DTXMessage>& message, const DTXMessageRoutingInfo& message_routing_info, Transmitter transmitter);
  uint32_t FragmentsForLength(size_t length);
  
  void SetSuggestedFragmentSize(uint32_t suggested_fragment_size) { suggested_fragment_size_ = suggested_fragment_size; }
  uint32_t SuggestedFragmentSize() const { return suggested_fragment_size_; }
  
 private:
  uint32_t suggested_fragment_size_ = 64 * 1024; // 0x10000(64kb), include the size of the header
  
}; // class DTXMessageTransmitter

}  // namespace idevice

#endif // IDEVICE_DTXMESSAGE_TRANSMITTER_H
