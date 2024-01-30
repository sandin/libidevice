#include "argparser.hpp"

#include <cstring> // strlen

using namespace idevice::tools;

idevice::tools::Args idevice::tools::parse_args(int argc, char* argv[]) {
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

bool idevice::tools::is_flag_set(const Args& args, const std::string& key) {
  return args.second.find(key) != args.second.end();
}

int idevice::tools::get_flag_as_int(const Args& args, const std::string& key, int def_val) {
  auto found = args.second.find(key);
  if (found != args.second.end()) {
    if (!found->second.empty()) {
      return std::stoi(found->second);
    }
  }
  return def_val;
}
