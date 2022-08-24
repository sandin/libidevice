#include "idevice/dtxmessageparser.h"

using namespace idevice;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  DTXMessageParser parser;
  bool ret = parser.ParseIncomingBytes(reinterpret_cast<const char*>(data), size);
  return 0; 
}
