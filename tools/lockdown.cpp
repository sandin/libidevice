#include "lockdown.hpp"

#include <string>
#include <thread>

#include "idevice/service/lockdownservice.h"
#include "nskeyedarchiver/scope.hpp"  // nskeyedarchiver::make_scope_exit

using namespace idevice;

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

int idevice::tools::lockdown_get_value(const idevice::tools::Args& args) {
  if (args.first.size() < 2) {
    printf("missing args, USAGE: get_value <domain> <key>\n");
    return -1;
  }
  std::string domain = args.first.at(0);
  std::string key = args.first.at(1);

  char** devices;
  int count = 0;
  int ret = idevice_get_device_list(&devices, &count);
  if (ret != IDEVICE_E_SUCCESS || count == 0) {
    printf("no connected device!\n");
    return -1;
  }
  for (int i = 0; i < count; ++i) {
    char* device_udid = devices[i];
    printf("%d UDID: %s\n", i, device_udid);
  }
  std::string udid = devices[0];
  idevice_device_list_free(devices);

  idevice_t device = nullptr;
  idevice_new_with_options(&device, udid.c_str(), IDEVICE_LOOKUP_USBMUX);
  if (device == nullptr) {
    printf("Can not create a new device(udid: %s).\n", udid.c_str());
    return -1;
  }
  defer(device, idevice_free(device));

  LockdownService* lockdown_service = new LockdownService();
  defer(lockdown_service, delete lockdown_service);

  lockdown_service->Connect(device);
  defer(lockdown_service_c, lockdown_service->Disconnect());

  std::string val = lockdown_service->GetValue(domain.c_str(), key.c_str(), "");
  printf("%s: %s\n", key.c_str(), val.c_str());

  return ret;
}
