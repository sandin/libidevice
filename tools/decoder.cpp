#include <sys/stat.h>  // stat
#include <cstdio>   // fopen
#include <cstring>  // strcmp
#include <iostream>
#include <string>
#include <utility> // std::pair
#include <vector>
#include <unordered_map>
#include <algorithm> // std::sort

#include "idevice/dtxmessageparser.h"
#include "nskeyedarchiver/nskeyedunarchiver.hpp"

#define USAGE                                              \
  "Usage: idevice_decoder [options] <dtx_message_dump_files..>\n" \
  "  options:\n" \
  "    `--hex`: dump data as hex string\n" \
  "    `--limit [count]`: parse messages limit number pre file\n"


using namespace idevice;
using Args = std::pair<std::vector<std::string>, std::unordered_map<std::string, std::string>>;

static Args parse_args(int argc, char* argv[]) {
  Args params;
  std::string key;
  for (int i = 1; i < argc; ++i) {
    char* arg = argv[i];
    size_t str_len = strlen(arg);
    if (str_len == 0) {
      key.clear();
      continue;
    } else if (str_len > 2 && arg[0] == '-') {
      key = arg + (arg[1] == '-' ? 2 : 1);
      params.second.insert(std::make_pair(key, ""));
    } else {
      if (!key.empty()) {
        params.second[key] = arg;
        key.clear();
      } else {
        params.first.push_back(arg);
      }
    }
  }
  return params;
}

static inline bool is_flag_set(const Args& args, const std::string& key) {
  return args.second.find(key) != args.second.end();
}

static int get_flag_as_int(const Args& args, const std::string& key, int def_val) {
  auto found = args.second.find(key);
  if (found != args.second.end()) {
    if (!found->second.empty()) {
      return std::stoi(found->second);
    }
  }
  return def_val;
}

std::vector<std::shared_ptr<DTXMessage>> decode_dtxmsg_dump_file(const std::string& filename, int limit, bool dumphex) {
  FILE* f = fopen(filename.c_str(), "rb");
  if (!f) {
    printf("can not open `%s` file.\n", filename.c_str());
    return std::vector<std::shared_ptr<DTXMessage>>(); // empty vector
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
  
  return parser.PopAllParsedMessages(); // copy all pointers
}

int decode_dtxmsg_dump_files(const std::vector<std::string>& filenames, int limit, bool dumphex) {
  std::vector<std::shared_ptr<DTXMessage>> messages;
  for (const auto& filename : filenames) {
    auto subvec = decode_dtxmsg_dump_file(filename, limit, dumphex);
    messages.insert(messages.end(), subvec.begin(), subvec.end()); // append_all
  }
  
  std::sort(messages.begin(), messages.end(), [](const auto& a, const auto& b) {
    // order by msgid and cidx
    if (a ->Identifier() != b->Identifier()) {
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

int main(int argc, char* argv[]) {
  Args args = parse_args(argc, argv);
  if (args.first.size() < 1) {
    printf("%s", USAGE);
    return 0;
  }
  std::vector<std::string> dump_file = args.first;
  bool dumphex = is_flag_set(args, "hex");
  int limit = get_flag_as_int(args, "limit", -1);
  decode_dtxmsg_dump_files(dump_file, limit, dumphex);

  return 0;
}
