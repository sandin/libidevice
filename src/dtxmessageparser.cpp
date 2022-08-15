#include "idevice/dtxmessageparser.h"


using namespace idevice;

#define DUMP_DTX_MESSAGE_HEADER(header) \
  printf("==============\n"); \
  printf("magic: %x\n", header.magic); \
  printf("message_header_size: %d\n", header.message_header_size); \
  printf("fragment_id: %d\n", header.fragment_id); \
  printf("fragment_count: %d\n", header.fragment_count); \
  printf("length: %d\n", header.length); \
  printf("identifier: %d\n", header.identifier); \
  printf("conversation_index: %d\n", header.conversation_index); \
  printf("channel_code: %d\n", header.channel_code); \
  printf("expects_reply: %d\n", header.expects_reply); \
  printf("==============\n"); \


static const uint32_t kDTXMessageHeaderMagic = 0x1F3D5B79;
static const uint32_t kDTXMessageHeaderSize = sizeof(DTXMessageHeader);

// run on worker thread
bool DTXMessageParser::ParseIncomingBytes(const char* buffer, size_t size) {
  // copy the data from receive buffer into parsing buffer
  {
    hexdump((void*)buffer, (int)size, 0); // FIXME: debug only
    char* ptr = parsing_buffer_.Allocate(size);
    if (ptr == nullptr) {
      printf("Error: can not parse incoming bytes, OOM.\n");
      return;
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
    const char* ptr = parsing_buffer_.GetPtr(consumed_size);
    DTXMessageHeader header;
    header.magic = *(uint32_t*)ptr; // assume little endian
    header.message_header_size = *(uint32_t*)(ptr + 4);
    header.fragment_id = *(uint16_t*)(ptr + 8);
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
    if (size < consumed_size + message_size_with_header) {
      // Case B, we got a complete DTXMessageHeader with a partial DTXMessage payload,
      // we just leave the complete header in the buffer, and do nothing but just return,
      // wait for the next time when the buffer is filled with more received data
      break;
    }
    
    // Case C, we got at least one complete header and payload, parse(and consume) them
    ParseMessageWithHeader(header, ptr + kDTXMessageHeaderSize, header.length);
    consumed_size += message_size_with_header;
  } // end of while
  
  assert(consumed_size <= buffer_size); // TODO: define DEBUG_ASSERT
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
  DUMP_DTX_MESSAGE_HEADER(header);
  
  std::unique_ptr<DTXMessage> message = DTXMessage::Create(data, size);
  
  if (header.fragment_count == 1) {
    // DTXMessage has only one fragment
    // TODO:
    
  } else {
    // DTXMessage has multiple fragments
    
    // TODO:
  }
  
  return false; // FIXME:
}

