#include "idevice/dtxmessageparser.h"

#include "idevice/macro_def.h"

using namespace idevice;

// run on worker thread
bool DTXMessageParser::ParseIncomingBytes(const char* buffer, size_t size) {
  // copy the data from receive buffer into parsing buffer
  {
#if IDEVICE_DEBUG
    hexdump((void*)buffer, (int)size, 0);
#endif
    char* ptr = parsing_buffer_.Allocate(size);
    if (ptr == nullptr) {
      printf("Error: can not parse incoming bytes, OOM.\n");
      return false;
    }
    memcpy(ptr, buffer, size);
  }
  
  // At this point the contents `parsing_buffer` can contain:
  // A) An incomplete DTXMessageHeader
  //   [heade]          (missing some bytes of header)
  //   ^
  //   ^
  // B) A complete DTXMessageHeader + An incomplete DTXMessage
  //   [header][payloa] (missing some bytes of payload)
  //   ^
  //   ^
  // C) One or more complete DTXMessageHeader+DTXMessage
  //   [header][payload]
  //   ^
  //                    ^
  // D) Some complete DTXMessageHeader+DTXMessage and a partial DTXMessageHeader or DTXMessage (C + A/B)
  //   [header][payload][heade]              (C + A)
  //   ^
  //                    ^
  //   [header][payload][header][paylod]     (C + B)
  //   ^
  //                    ^
  // we will parse as much data in the buffer as possible, after this function return, the `parsing_buffer_` is
  // either empty, we parsed all the messages(see `^` in Case C)
  // or starts with the header of the next message, still incomplete.(see `^` in Case A/B/D)
  size_t consumed_size = 0; // offset of parsing buffer
  size_t buffer_size = parsing_buffer_.Size();
  while (true) {
    if (buffer_size < consumed_size + kDTXMessageHeaderSize) {
      break; // Case A, not enough data to read even an DTXMessageHeader
    }
    
    // whatever the case(B/C/D), we have at least one complete header, read(not consume) it
    // DTXMessage Memory Layout:
    // |-----------------------------------------------------------|
    // |  0  1  2  3  |  4  5  6  7  |  8  9  A  B  |  C  D  E  F  |
    // |-----------------------------------------------------------|
    // | magic        | header_size  | fidx  | fcnt | length       | // `DTXMessageHeader header`, header of DTXMessage, size=0x20
    // | identifier   | conv_idx     | channel_code | expects_reply|
    // | ...                                                       | // `DTXMessagePayload payload`, payload of DTXMessage
    // |-----------------------------------------------------------|
    const char* ptr = parsing_buffer_.GetPtr(consumed_size);
#if IDEVICE_DEBUG
    hexdump((void*)ptr, (int)kDTXMessageHeaderSize, 0);
#endif
    DTXMessageHeader header;
    header.magic = *(uint32_t*)ptr; // assume little endian
    header.message_header_size = *(uint32_t*)(ptr + 4);
    header.fragment_index = *(uint16_t*)(ptr + 8);
    header.fragment_count = *(uint16_t*)(ptr + 10);
    header.length = *(uint32_t*)(ptr + 12);
    header.identifier = *(uint32_t*)(ptr + 16);
    header.conversation_index = *(uint32_t*)(ptr + 20);
    header.channel_code = *(uint32_t*)(ptr + 24);
    header.expects_reply = *(uint32_t*)(ptr + 28);
    ptr += kDTXMessageHeaderSize;
    if (header.magic != kDTXMessageHeaderMagic) {
      printf("Error: handling %zu bytes with unexpected protocol header(magic=%d).\n", size, header.magic);
      return false;
    }
    if (header.message_header_size != kDTXMessageHeaderSize) {
      printf("Error: handling %zu bytes with unexpected protocol header(header_size=%d).\n", size, header.message_header_size);
      return false;
    }
    
    size_t message_size_with_header = header.message_header_size + header.length;
    if (buffer_size < consumed_size + message_size_with_header) {
      // Case B, we got a complete DTXMessageHeader with a partial DTXMessage payload,
      // we just leave the complete header in the buffer, and do nothing but just return,
      // wait for the next time when the buffer is filled with more received data
      break;
    }
    
    // Case C, we got at least one complete header and payload, parse(and consume) them
    ParseMessageWithHeader(header, ptr, header.length);
    consumed_size += message_size_with_header;
  } // end of while
  
  IDEVICE_ASSERT(consumed_size <= buffer_size, "consumed_size <= buffer_size");
  if (consumed_size > 0) {
    // Case D, after we consume one or more complete DTXMessageHeader+DTXMessage,
    // but there is a leftover at the end of the buffer.
    // Shift out the consumed bytes, so that on the next time `parsing_buffer_` starts with
    // the header of the next unconsumed message.
    buffer_size -= consumed_size;
    if (buffer_size > 0) {
      const char* fresh_start = parsing_buffer_.GetPtr(consumed_size);
      memmove(parsing_buffer_.GetPtr(0), fresh_start, buffer_size);
      parsing_buffer_.SetSize(buffer_size);
    } // else: Case C, which there is nothing left in the buffer, so we don't neet to move any memory either
  }
  return true;
}

bool DTXMessageParser::ParseMessageWithHeader(const DTXMessageHeader& header, const char* data, size_t size) {
#if IDEVICE_DEBUG
  IDEVICE_DUMP_DTXMESSAGE_HEADER(header);
#endif
  
  if (header.fragment_count == 1) {
    // DTXMessage has only one fragment
    // TODO:
    
    std::shared_ptr<DTXMessage> message = DTXMessage::Deserialize(data, size);
    message->SetIdentifier(header.identifier);
    message->SetConversationIndex(header.conversation_index);
    message->SetChannelCode(header.channel_code);
    message->SetExpectsReply(header.expects_reply != 0);
    parsed_message_queue_.emplace(std::move(message));

  } else {
    // DTXMessage has multiple fragments
    
    printf("TODO: decode multiple fragments\n");
    
    // TODO:
  }
  
  return false; // FIXME:
}

std::vector<std::shared_ptr<DTXMessage>> DTXMessageParser::PopAllParsedMessages() {
  std::vector<std::shared_ptr<DTXMessage>> result;
  while (!parsed_message_queue_.empty()) {
    std::shared_ptr<DTXMessage> msg = parsed_message_queue_.front();
    result.emplace_back(std::move(msg));
    parsed_message_queue_.pop();
  }
  return result; // moved
}

