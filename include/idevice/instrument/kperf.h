#ifndef IDEVICE_INSTRUMENT_KTRACE_H
#define IDEVICE_INSTRUMENT_KTRACE_H

#include <cstdint>
#include <vector>
#include <set>
#include <string>

#include "nskeyedarchiver/kavalue.hpp"
#include "nskeyedarchiver/kaarray.hpp"
#include "nskeyedarchiver/kamap.hpp"

namespace idevice {

using KDebugId = uint32_t;

class DTKTraceTapTriggerConfig {
 public:
  DTKTraceTapTriggerConfig()
    : fields_(nskeyedarchiver::KAMap("NSMutableDictionary", {"NSMutableDictionary", "NSDictionary", "NSObject"})) {}

  void SetKind(uint32_t kind) {
    fields_["tk"] = kind;
  }
  
  void SetUUID(const char* uuid) {
    fields_["uuid"] = nskeyedarchiver::KAMap("NSMutableString", {"NSMutableString", "NSString", "NSObject"},
                                            {{"NS.string", nskeyedarchiver::KAValue(uuid)}});
  }
  
  void SetKDebugFilter(std::set<KDebugId> debug_codes) {
    nskeyedarchiver::KAArray codes("NSMutableSet", {"NSMutableSet", "NSSet", "NSObject"});
    for (KDebugId debug_id : debug_codes) {
      codes.push_back(nskeyedarchiver::KAValue(debug_id));
    }
    fields_["kdf2"] = std::move(codes);
  }
  
  nskeyedarchiver::KAValue ToKAValue() const { return nskeyedarchiver::KAValue(fields_); /* copy */ }
 private:
  nskeyedarchiver::KAMap fields_;
};

class DTKTraceTapConfig {
 public:
  DTKTraceTapConfig() : fields_(nskeyedarchiver::KAMap("NSDictionary", {"NSDictionary", "NSObject"})) {}
  
  void SetBufferMode(uint32_t buffer_mode) { fields_["bm"] = buffer_mode; }
  void SetRecodingPriority(uint32_t recoding_priority) { fields_["rp"] = recoding_priority; }
  void SetPollingInterval(uint64_t polling_interval)  { fields_["ur"] = polling_interval; }
  
  void SetTriggerConfig(const DTKTraceTapTriggerConfig& config) {
    nskeyedarchiver::KAArray configs("NSArray", {"NSArray", "NSObject"});
    configs.push_back(config.ToKAValue());
    fields_["tc"] = std::move(configs);
  }
  
  nskeyedarchiver::KAValue ToKAValue() const { return nskeyedarchiver::KAValue(fields_); /* copy */ }
 private:
  nskeyedarchiver::KAMap fields_;
};

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_KTRACE_H
