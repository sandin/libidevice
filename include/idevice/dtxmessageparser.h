#ifndef IDEVICE_DTXMESSAGE_PARSER_H
#define IDEVICE_DTXMESSAGE_PARSER_H

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
  
 private:
  //const char* Read(ByteReader& reader, size_t size, size_t* actual_size);
  bool ParseMessageWithHeader(const DTXMessageHeader& header, const char* data, size_t size);
  
  bool eof_ = false;
  BufferMemory parsing_buffer_;
}; // class DTXMessageParser

}  // namespace idevice

#endif // IDEVICE_DTXMESSAGE_PARSER_H
