#ifndef IDEVICE_TOOLS_ARGPARSER_H
#define IDEVICE_TOOLS_ARGPARSER_H

#include <algorithm>  // std::sort
#include <utility>  // std::pair
#include <vector>
#include <string>
#include <unordered_map>

namespace idevice { 
namespace tools {

using Args = std::pair<std::vector<std::string>, std::unordered_map<std::string, std::string>>;

Args parse_args(int argc, char* argv[]);
bool is_flag_set(const Args& args, const std::string& key);
int get_flag_as_int(const Args& args, const std::string& key, int def_val);

} // namespace tools
} // namespace idevice

#endif // IDEVICE_TOOLS_ARGPARSER_H
