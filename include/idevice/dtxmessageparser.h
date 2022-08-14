#ifndef IDEVICE_DTXMESSAGE_PARSER_H
#define IDEVICE_DTXMESSAGE_PARSER_H

#include "idevice/idevice.h"
#include "idevice/dtxmessage.h"
#include "idevice/bytebuffer.h"

namespace idevice {

class DTXMessageParser {
 public:
  DTXMessageParser() : parsing_buffer_(128) {}
  virtual ~DTXMessageParser() {}
  
  void ParseIncomingBytes(const char* buffer, size_t size);
  
 private:
  const char* ReadMessage(ByteReader& reader, size_t size, size_t* actual_size);
  
  bool eof_ = false;
  ByteBuffer parsing_buffer_;
  
}; // class DTXMessageParser

}  // namespace idevice

#endif // IDEVICE_DTXMESSAGE_PARSER_H
