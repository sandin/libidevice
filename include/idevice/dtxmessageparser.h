#ifndef IDEVICE_DTXMESSAGE_PARSER_H
#define IDEVICE_DTXMESSAGE_PARSER_H

#include <queue>
#include <vector>

#include "idevice/idevice.h"
#include "idevice/dtxmessage.h"
#include "idevice/bytebuffer.h"

namespace idevice {

class DTXMessageParser {
 public:
  DTXMessageParser() {}
  
  virtual ~DTXMessageParser() {
  }
  
  bool ParseIncomingBytes(const char* buffer, size_t size);
  
  std::vector<std::shared_ptr<DTXMessage>> PopAllParsedMessages();
  
 private:
  //const char* Read(ByteReader& reader, size_t size, size_t* actual_size);
  bool ParseMessageWithHeader(const DTXMessageHeader& header, const char* data, size_t size);
  
  bool eof_ = false;
  BufferMemory parsing_buffer_;
  std::queue<std::shared_ptr<DTXMessage>> parsed_message_queue_;
}; // class DTXMessageParser

}  // namespace idevice

#endif // IDEVICE_DTXMESSAGE_PARSER_H
