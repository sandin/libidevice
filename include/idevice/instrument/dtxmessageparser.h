#ifndef IDEVICE_INSTRUMENT_DTXMESSAGE_PARSER_H
#define IDEVICE_INSTRUMENT_DTXMESSAGE_PARSER_H

#include <queue>
#include <unordered_map>
#include <vector>

#include "idevice/common/idevice.h"
#include "idevice/instrument/dtxmessage.h"
#include "idevice/utils/bytebuffer.h"

namespace idevice {

/**
 * A Parser for the DTXMessage
 * It is responsible for parsing binary data from the server into DTXMessage.
 */
class DTXMessageParser {
 public:
  /**
   * Constructor
   */
  DTXMessageParser() {}

  /**
   * Destructor
   */
  virtual ~DTXMessageParser() {}

  /**
   * parse incoming data
   *
   * @param buffer the incoming data
   * @param size  size of the data
   * @return succeed or fail
   */
  bool ParseIncomingBytes(const char* buffer, size_t size);

  /**
   * Pop all parsed messages
   *
   * @return std::vector<std::shared_ptr<DTXMessage>>
   */
  std::vector<std::shared_ptr<DTXMessage>> PopAllParsedMessages();

  /**
   * Get the size of parsed messagees
   *
   * @return size_t the size of parsed messages
   */
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

#endif  // IDEVICE_INSTRUMENT_DTXMESSAGE_PARSER_H
