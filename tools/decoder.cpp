#include <sys/stat.h>  // stat

#include <cstdio>   // fopen
#include <cstring>  // strcmp
#include <iostream>
#include <string>

#include "idevice/dtxmessageparser.h"
#include "nskeyedarchiver/nskeyedunarchiver.hpp"

#define USAGE                                              \
  "Usage: idevice_decoder <dtx_message_dump_file>\n" \

using namespace idevice;

int decode_dtxmsg_dump_file(const char* filename, bool dumphex) {
  FILE* f = fopen(filename, "rb");
  if (!f) {
    printf("can not open `%s` file.\n", filename);
    return -1;
  }
  
  DTXMessageParser parser;
  uint32_t msg_count = 0;

  size_t buffer_size = 8 * 1024;
  char* buffer = static_cast<char*>(malloc(buffer_size));
  int actual = 0;
  size_t count = 0;
  while (true) {
    actual = fread(buffer, 1, buffer_size, f);
    if (actual <= 0) {
      printf("EOF\n");
      break;
    }
    count += actual;
    
    bool ret = parser.ParseIncomingBytes(buffer, actual);
    if (!ret) {
      printf("ret=%d\n", ret);
      break;
    }
  }
  free(buffer);
  fclose(f);
  
  std::vector<std::shared_ptr<DTXMessage>> messages = parser.PopAllParsedMessages();
  msg_count += messages.size();
  for (const auto& msg : messages) {
    printf("msg_id: %d\n", msg->Identifier());
    msg->Dump(dumphex);
    printf("\n");
  }

  printf("read %zu bytes from `%s`, decoded %d messages.\n", count, filename, msg_count);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("%s", USAGE);
    return 0;
  }
  char* dump_file = argv[1];
  decode_dtxmsg_dump_file(dump_file, false);

  return 0;
}
