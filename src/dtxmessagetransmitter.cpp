#include "idevice/dtxmessagetransmitter.h"

#include <cmath> // ceil

#include "idevice/macro_def.h"

#define DUMP_DTX_MESSAGE_HEADER(header) \
  printf("==============\n"); \
  printf("magic: %x\n", header.magic); \
  printf("message_header_size: %d\n", header.message_header_size); \
  printf("fragment_index: %d\n", header.fragment_index); \
  printf("fragment_count: %d\n", header.fragment_count); \
  printf("length: %d\n", header.length); \
  printf("identifier: %d\n", header.identifier); \
  printf("conversation_index: %d\n", header.conversation_index); \
  printf("channel_code: %d\n", header.channel_code); \
  printf("expects_reply: %d\n", header.expects_reply); \
  printf("==============\n"); \

using namespace idevice;

bool DTXMessageTransmitter::TransmitMessage(const std::shared_ptr<DTXMessage>& message, uint32_t fragment_index, DTXMessageRoutingInfo routing_info, Transmitter transmitter) {
  size_t serialized_length = message->SerializedLength();
  uint32_t number_of_pieces = FragmentsForLength(serialized_length);
  assert(fragment_index < number_of_pieces); // TODO: DEBUG_ASSERT("fragmentIndex < numberOfPieces")
  
  if (number_of_pieces == 1) {
    // single fragment
    assert(fragment_index == 0); // TODO: DEBUG_ASSERT("fragmentIndex == 0")
    
    // transmit the DTXMessageHeader, size=0x20
    DTXMessageHeader header;
    header.magic = kDTXMessageHeaderMagic;
    header.message_header_size = kDTXMessageHeaderSize;
    header.fragment_index = fragment_index;
    header.fragment_count = number_of_pieces;
    header.length = serialized_length;
    header.identifier = message->Identifier(); // TODO
    header.conversation_index = message->ConversationIndex(); // TODO
    header.channel_code = message->ChannelCode(); // TODO
    header.expects_reply = 1; // TODO
    IDEVICE_LOG_D("TransmitMessage message\n");
    DUMP_DTX_MESSAGE_HEADER(header);
    transmitter(reinterpret_cast<const char*>(&header), sizeof(DTXMessageHeader));
    
    // transmit the DTXMessage(payload)
#if IDEVICE_DEBUG
    message->Dump();
#endif
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

