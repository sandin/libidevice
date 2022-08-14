#include "idevice/dtxmessageparser.h"


using namespace idevice;

#define DUMP_DTX_MESSAGE_HEADER(header) \
  printf("==============\n"); \
  printf("magic: %x\n", header->magic); \
  printf("message_header_size: %d\n", header->message_header_size); \
  printf("fragment_id: %d\n", header->fragment_id); \
  printf("fragment_count: %d\n", header->fragment_count); \
  printf("length: %d\n", header->length); \
  printf("identifier: %d\n", header->identifier); \
  printf("conversation_index: %d\n", header->conversation_index); \
  printf("channel_code: %d\n", header->channel_code); \
  printf("expects_reply: %d\n", header->expects_reply); \
  printf("==============\n"); \

static const uint32_t kDTXMessageHeaderMagic = 0x1F3D5B79;

static inline void hexdump(void* addr, int len, int offset) {
  int i;
  unsigned char buff[17];
  unsigned char* pc = (unsigned char*)addr;

  // Process every byte in the data.
  for (i = 0; i < len; i++) {
    // Multiple of 16 means new line (with line offset).

    if ((i % 16) == 0) {
      // Just don't print ASCII for the zeroth line.
      if (i != 0) printf("  %s\n", buff);

      // Output the offset.
      printf("  %08x ", i + offset);
    }

    // Now the hex code for the specific character.
    printf(" %02x", pc[i]);

    // And store a printable ASCII character for later.
    if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
      buff[i % 16] = '.';
    } else {
      buff[i % 16] = pc[i];
    }

    buff[(i % 16) + 1] = '\0';
  }

  // Pad out last line if not exactly 16 characters.
  while ((i % 16) != 0) {
    printf("   ");
    i++;
  }

  // And print the final ASCII bit.
  printf("  %s\n", buff);
}

void DTXMessageParser::ParseIncomingBytes(const char* buffer, size_t size) {
  ByteReader reader(buffer, size);
  while (reader.HasMore()) {
    size_t actual_size;
    const char* data = ReadMessage(reader, sizeof(DTXMessageHeader), &actual_size);
    if (!data || eof_) {
      return; // break;
    }
    const DTXMessageHeader* header = reinterpret_cast<const DTXMessageHeader*>(data);
    if (header->magic != kDTXMessageHeaderMagic) {
      printf("Error: wrong magic number.\n");
      break;
    }
    if (header->fragment_count == 1) {
      // single fragment
      const char* d = ReadMessage(reader, header->length, &actual_size);
      
    } else {
      // multiple fragments
      
    }
    
    
  }
  
  DTXMessageHeader* header = (DTXMessageHeader*)buffer;
  DUMP_DTX_MESSAGE_HEADER(header);
  
  hexdump((void*)(buffer + header->message_header_size), (int)header->length, 0);
  
  // TODO:
}

const char* DTXMessageParser::ReadMessage(ByteReader& reader, size_t size, size_t* actual_size) {
  size_t size_read = 0;
  const char* ptr = reader.Read(size, &size_read);
  parsing_buffer_.Append(ptr, size_read);  // copy data from receive buffer to parsing buffer
  if (size_read >= size) {
    *actual_size = size_read;
    return reinterpret_cast<const char*>(parsing_buffer_.GetReadOnlyBuffer(parsing_buffer_.Size() - size_read));
  }
  *actual_size = 0;
  return nullptr;
}

