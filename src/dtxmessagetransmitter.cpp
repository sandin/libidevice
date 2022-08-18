#include "idevice/dtxmessagetransmitter.h"

#include <cmath> // ceil

using namespace idevice;

bool DTXMessageTransmitter::TransmitMessage(const std::shared_ptr<DTXMessage>& message, uint32_t fragment_index, DTXMessageRoutingInfo message_routing_info, Transmitter transmitter) {
  size_t serialized_length = message->SerializedLength();
  uint64_t number_of_pieces = FragmentsForLength(serialized_length);
  assert(fragment_index < number_of_pieces); // TODO: DEBUG_ASSERT("fragmentIndex < numberOfPieces")
  
  if (number_of_pieces == 1) {
    // single fragment
    assert(fragment_index > 0); // TODO: DEBUG_ASSERT("fragmentIndex == 0")
    
    // transmit the DTXMessageHeader, size=0x20
    DTXMessageHeader header;
    header.magic = kDTXMessageHeaderMagic;
    header.message_header_size = kDTXMessageHeaderSize;
    header.fragment_index = fragment_index;
    header.fragment_count = number_of_pieces;
    header.length = serialized_length;
    header.identifier = message_routing_info.identifier;
    header.conversation_index = message_routing_info.conversation_index;
    header.channel_code = 0; // TODO
    header.expects_reply = 0; // TODO
    transmitter(reinterpret_cast<const char*>(&header), sizeof(DTXMessageHeader));
    
    // transmit the DTXMessage(payload)
    message->SerializeTo(transmitter);
    return true;
  } else {
    // multiple fragments
    
    // TODO:
    return true;
  }
}

uint32_t DTXMessageTransmitter::FragmentsForLength(size_t length) {
  uint32_t fragments_count = 1;
  if (suggested_fragment_size_ >= kDTXMessageHeaderSize + 1) {
    fragments_count = ceil(length / (suggested_fragment_size_ - kDTXMessageHeaderSize));
  }
  return fragments_count;
}

