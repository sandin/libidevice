#ifndef IDEVICE_INSTRUMENT_DTXMESSAGE_TRANSMITTER_H
#define IDEVICE_INSTRUMENT_DTXMESSAGE_TRANSMITTER_H

#include "idevice/common/idevice.h"
#include "idevice/instrument/dtxmessage.h"

namespace idevice {

/**
 * A Transmitter for the DTXMessage 
 * It is responsible for serializing the DTXMessage into binary data and transmiting it to the server.
 */
class DTXMessageTransmitter {
 public:
  using Transmitter = std::function<bool(const char*, size_t)>;
  
  /**
   * Constructor
   */
  DTXMessageTransmitter() {}

  /**
   * Destructor
   */
  virtual ~DTXMessageTransmitter() {}
  
  /**
   * Transmit a message
   * 
   * @param message the message
   * @param message_routing_info the routing info of the message 
   * @param transmitter transmitter
   * @return succeed or fail
   */
  bool TransmitMessage(const std::shared_ptr<DTXMessage>& message, const DTXMessageRoutingInfo& message_routing_info, Transmitter transmitter);

  /**
   * Get the count of fragments for the length of the message
   * If a message is too long, we need to split it into multiple fragments for transmission.
   * 
   * @param length the length of the message
   * @return uint32_t the count of fragments
   */
  uint32_t FragmentsForLength(size_t length);
  
  /**
   * Set the suggested fragment size
   * @param suggested_fragment_size 
   */
  void SetSuggestedFragmentSize(uint32_t suggested_fragment_size) { suggested_fragment_size_ = suggested_fragment_size; }

  /**
   * Get the suggested fragment size 
   * 
   * @return uint32_t the suggested fragment size
   */
  uint32_t SuggestedFragmentSize() const { return suggested_fragment_size_; }
  
 private:
  uint32_t suggested_fragment_size_ = 64 * 1024; // 0x10000(64kb), include the size of the header
  
}; // class DTXMessageTransmitter

}  // namespace idevice

#endif // IDEVICE_INSTRUMENT_DTXMESSAGE_TRANSMITTER_H
