#include "idevice/dtxmessagetransmitter.h"

#include <cmath> // ceil

#include "idevice/macro_def.h"

using namespace idevice;

bool DTXMessageTransmitter::TransmitMessage(const std::shared_ptr<DTXMessage>& message, uint32_t fragment_index, const DTXMessageRoutingInfo& routing_info, Transmitter transmitter) {
  size_t serialized_length = message->SerializedLength();
  uint32_t number_of_pieces = FragmentsForLength(serialized_length);
  IDEVICE_ASSERT(fragment_index < number_of_pieces, "fragmentIndex < numberOfPieces");
  
  if (number_of_pieces == 1) {
    // single fragment
    IDEVICE_ASSERT(fragment_index == 0, "fragmentIndex == 0");
    
    // transmit the DTXMessageHeader, size=0x20
    DTXMessageHeader header;
    header.magic = kDTXMessageHeaderMagic;
    header.message_header_size = kDTXMessageHeaderSize;
    header.fragment_index = fragment_index;
    header.fragment_count = number_of_pieces;
    header.length = serialized_length;
    header.identifier = routing_info.msg_identifier;
    header.conversation_index = routing_info.conversation_index;
    header.channel_code = routing_info.channel_code;
    header.expects_reply = routing_info.expects_reply;
#if IDEVICE_DEBUG
    IDEVICE_LOG_D("TransmitMessage message\n");
    IDEVICE_DUMP_DTXMESSAGE_HEADER(header);
#endif
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
    fragments_count = ceil(length / (float)(suggested_fragment_size_ - kDTXMessageHeaderSize));
  }
  return fragments_count;
}

