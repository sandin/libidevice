#include "amfi.hpp"
#include "argparser.hpp"
#include "decoder.hpp"
#include "instruments.hpp"
#include "lockdown.hpp"

using namespace idevice::tools;

#define USAGE                                                                                    \
  "Usage: idevice [command]\n"                                                                   \
  "commands:\n"                                                                                  \
  " decode [options] <dtx_message_dump_files..>           decode the binary DTXMessages files\n" \
  "     --hex: dump data as hex string\n"                                                        \
  "     --limit [count]: parse messages limit number pre file\n"                                 \
  " instruments [subcommand]\n"                                                                  \
  "   running_processes                                    print the running processes\n"

int main(int argc, char* argv[]) {
  Args args = parse_args(argc, argv);
  if (args.first.size() < 1) {
    printf("%s", USAGE);
    return 0;
  }

  std::string command = args.first.at(0);
  args.first.erase(args.first.begin());
  if (command == "decode") {
    return decoder_main(args);
  } else if (command == "instruments") {
    return instruments_main(args);
  } else if (command == "get_value") {
    return lockdown_get_value(args);
  } else if (command == "set_developer_mode") {
    return amfi_set_developer_mode(args);
  } else {
    printf("unknown command: %s\n", command.c_str());
  }
  return 0;
}
