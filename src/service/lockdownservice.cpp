#include "idevice/service/lockdownservice.h"

#include <functional>  // std::bind, std::placeholders

#include "idevice/common/macro_def.h"
#include "plist/plist.h"

using namespace idevice;

IService::Result LockdownService::Connect(idevice_t device) {
  lockdownd_error_t ret = lockdownd_client_new_with_handshake(device, &client_, kClientLabel);
  return ret;
}

IService::Result LockdownService::Disconnect() {
  if (client_ != nullptr) {
    lockdownd_client_free(client_);
    client_ = nullptr;
  }
  return LOCKDOWN_E_SUCCESS;
}

std::string LockdownService::GetValue(const char* domain, const char* key,
                                      const std::string def_val) {
  IDEVICE_ASSERT(client_ != nullptr, "client_ can not be null.");
  plist_t value;
  lockdownd_error_t ret = lockdownd_get_value(client_, domain, key, &value);
  if (ret != LOCKDOWN_E_SUCCESS) {
    return def_val;
  }

  switch (plist_get_node_type(value)) {
    case PLIST_BOOLEAN: {
      uint8_t val = 0;
      plist_get_bool_val(value, &val);
      return std::to_string(val);
    }
    // TODO: other types
    default:
      return def_val;
  }
}
