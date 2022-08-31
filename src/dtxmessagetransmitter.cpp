#include "idevice/dtxmessagetransmitter.h"

#include <algorithm>  // std::min
#include <cmath>      // ceil

#include "idevice/bytebuffer.h"
#include "idevice/idevice.h"  // hexdump
#include "idevice/macro_def.h"

using namespace idevice;

#if IDEVICE_DEBUG
#define IDEVICE_TRANSMIT_DUMP_HEADER(header)          \
  IDEVICE_LOG_D("TransmitMessage message header:\n"); \
  IDEVICE_DUMP_DTXMESSAGE_HEADER(header);
#else
#define IDEVICE_TRANSMIT_DUMP_HEADER(header)
#endif

// we use a buffer to cache all data of a fragment, when the buffer is full then flush all the data
// in it as one fragment of the DTXMessage. each framgent of the DTXMessage have their own header,
// and most of the values of these headers are the same, except for `length` and `fragment_index`.
#define IDEVICE_TRANSMIT_FRAGMENT_IF_BUFFER_FULL()                                                \
  if (buffer.Size() >= this_fragment_length) {                                                    \
    header.fragment_index += 1;                                                                   \
    header.length = this_fragment_length;                                                         \
    IDEVICE_TRANSMIT_DUMP_HEADER(header);                                                         \
    bool ret = transmitter(reinterpret_cast<const char*>(&header), sizeof(DTXMessageHeader));     \
    if (!ret) {                                                                                   \
      return false;                                                                               \
    }                                                                                             \
    IDEVICE_LOG_D("TransmitMessage message payload: ptr=%p, size=%zu\n",                          \
                  buffer.GetReadOnlyBuffer(0), buffer.Size());                                    \
    ret = transmitter(reinterpret_cast<const char*>(buffer.GetReadOnlyBuffer(0)), buffer.Size()); \
    serialized_offset += buffer.Size();                                                           \
    buffer.Resize(0);                                                                             \
    this_fragment_length = std::min(fragment_length, serialized_length - serialized_offset);      \
    if (!ret) {                                                                                   \
      return false;                                                                               \
    }                                                                                             \
  }

bool DTXMessageTransmitter::TransmitMessage(const std::shared_ptr<DTXMessage>& message,
                                            const DTXMessageRoutingInfo& routing_info,
                                            Transmitter transmitter) {
  const size_t serialized_length = message->SerializedLength();
  const uint32_t number_of_pieces = FragmentsForLength(serialized_length);

  DTXMessageHeader header;
  header.magic = kDTXMessageHeaderMagic;
  header.message_header_size = kDTXMessageHeaderSize;
  header.fragment_index = 0;
  header.fragment_count = number_of_pieces;
  header.length = serialized_length;
  header.identifier = routing_info.msg_identifier;
  header.conversation_index = routing_info.conversation_index;
  header.channel_code = routing_info.channel_code;
  header.expects_reply = routing_info.expects_reply;

  if (number_of_pieces == 1) {  // single fragment
    // transmit the DTXMessageHeader, size=0x20
    IDEVICE_TRANSMIT_DUMP_HEADER(header);
    transmitter(reinterpret_cast<const char*>(&header), sizeof(DTXMessageHeader));

    // transmit the DTXMessage(payload)
    message->SerializeTo(transmitter);
    return true;
  } else {  // multiple fragments
    // transmit the first fragment of the message, only contains the DTXMessageHeader, size=0x20
    IDEVICE_TRANSMIT_DUMP_HEADER(header);
    transmitter(reinterpret_cast<const char*>(&header), sizeof(DTXMessageHeader));

    const size_t fragment_length = suggested_fragment_size_ - kDTXMessageHeaderSize;
    ByteBuffer buffer(fragment_length);
    size_t serialized_offset = 0;
    size_t this_fragment_length = fragment_length;
    printf("this_fragment_length: %zu\n", this_fragment_length);

    message->SerializeTo([&](const char* bytes, size_t size) -> bool {
      IDEVICE_TRANSMIT_FRAGMENT_IF_BUFFER_FULL();
      size_t offset = 0;
      while (offset < size) {
        size_t consume_size =
            std::min({size, this_fragment_length - buffer.Size(), this_fragment_length});
        printf("offset=%zu, size=%zu, buffer.size=%zu, consume_size: %zu\n", offset, size,
               buffer.Size(), consume_size);
        buffer.Append(bytes + offset, consume_size);
        IDEVICE_TRANSMIT_FRAGMENT_IF_BUFFER_FULL();
        offset += consume_size;
      }
      return true;
    });
    return false;
  }
}

uint32_t DTXMessageTransmitter::FragmentsForLength(size_t length) {
  uint32_t fragments_count = 1;
  if (suggested_fragment_size_ >= kDTXMessageHeaderSize + 1) {
    fragments_count = ceil(length / (float)(suggested_fragment_size_ - kDTXMessageHeaderSize));
  }
  return fragments_count;
}

#undef IDEVICE_TRANSMIT_DUMP_HEADER
#undef IDEVICE_FLUSH_FRAGMENT_BUFFER
