#include "decoder.hpp"

#include <algorithm>  // std::sort
#include <cstdio>     // fopen
#include <cstring>    // strcmp
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>  // std::pair
#include <vector>

#include "idevice/instrument/dtxmessageparser.h"
#include "nskeyedarchiver/nskeyedunarchiver.hpp"

using namespace idevice;

static std::vector<std::shared_ptr<DTXMessage>> decode_dtxmsg_dump_file(const std::string& filename,
                                                                 int limit, bool dumphex) {
  FILE* f = fopen(filename.c_str(), "rb");
  if (!f) {
    printf("can not open `%s` file.\n", filename.c_str());
    return std::vector<std::shared_ptr<DTXMessage>>();  // empty vector
  }

  DTXMessageParser parser;

  size_t buffer_size = 8 * 1024;
  char* buffer = static_cast<char*>(malloc(buffer_size));
  int actual = 0;
  while (true) {
    if (limit != -1 && parser.ParsedMessageCount() >= limit) {
      printf("reach the limit: %d\n", limit);
      break;
    }

    actual = fread(buffer, 1, buffer_size, f);
    if (actual <= 0) {
      printf("EOF\n");
      break;
    }

    bool ret = parser.ParseIncomingBytes(buffer, actual);
    if (!ret) {
      printf("ret=%d\n", ret);
      break;
    }
  }
  free(buffer);
  fclose(f);

  return parser.PopAllParsedMessages();  // copy all pointers
}

int idevice::tools::decoder_main(const idevice::tools::Args& args) {
  std::vector<std::string> dump_file = args.first;
  bool dumphex = idevice::tools::is_flag_set(args, "hex");
  int limit = idevice::tools::get_flag_as_int(args, "limit", -1);

  std::vector<std::shared_ptr<DTXMessage>> messages;
  for (const auto& filename : dump_file) {
    printf("decode dtxmsg file: %s.\n", filename.c_str());
    auto subvec = decode_dtxmsg_dump_file(filename, limit, dumphex);
    messages.insert(messages.end(), subvec.begin(), subvec.end());  // append_all
  }

  std::sort(messages.begin(), messages.end(), [](const auto& a, const auto& b) {
    // order by msgid and cidx
    if (a->Identifier() != b->Identifier()) {
      return a->Identifier() < b->Identifier();
    } else {
      return a->ConversationIndex() < b->ConversationIndex();
    }
  });

  size_t msg_count = 0;
  for (const auto& msg : messages) {
    if (limit != -1 && msg_count >= limit) {
      printf("reach the limit: %d\n", limit);
      break;
    }

    printf("msg_id: %d\n", msg->Identifier());
    msg->Dump(dumphex);
    msg_count++;
    printf("\n");
  }

  printf("decoded %zu messages.\n", msg_count);
  return 0;
}
