#ifndef IDEVICE_DTXMESSAGE_PARSER_H
#define IDEVICE_DTXMESSAGE_PARSER_H

#include <queue>
#include <unordered_map>
#include <vector>

#include "idevice/bytebuffer.h"
#include "idevice/dtxmessage.h"
#include "idevice/idevice.h"

namespace idevice {

class DTXMessageParser {
 public:
  DTXMessageParser() {}
  virtual ~DTXMessageParser() {}

  bool ParseIncomingBytes(const char* buffer, size_t size);

  std::vector<std::shared_ptr<DTXMessage>> PopAllParsedMessages();
  size_t ParsedMessageCount() const { return parsed_message_queue_.size(); }

 private:
  // const char* Read(ByteReader& reader, size_t size, size_t* actual_size);
  size_t ParseMessageWithHeader(const DTXMessageHeader& header, const char* data, size_t size);

  bool eof_ = false;
  BufferMemory parsing_buffer_;
  std::unordered_map<uint32_t, ByteBuffer> fragmented_buffers_by_identifier;
  std::queue<std::shared_ptr<DTXMessage>> parsed_message_queue_;
};  // class DTXMessageParser

}  // namespace idevice

#endif  // IDEVICE_DTXMESSAGE_PARSER_H
