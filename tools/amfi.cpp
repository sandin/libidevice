#include "amfi.hpp"

#include <string>
#include <thread>

#include "idevice/service/amfiservice.h"
#include "nskeyedarchiver/scope.hpp"  // nskeyedarchiver::make_scope_exit

using namespace idevice;

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

int idevice::tools::amfi_set_developer_mode(const idevice::tools::Args& args) {
  if (args.first.size() < 1) {
    printf("missing args, USAGE: set_developer_mode <mode>\n");
    return -1;
  }
  int mode = std::stoi(args.first.at(0));

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

  AMFIService* amfi_service = new AMFIService();
  defer(amfi_service, delete amfi_service);

  ret = amfi_service->Connect(device);
  if (ret != IDEVICE_E_SUCCESS) {
    return ret;
  }
  defer(amfi_service_c, amfi_service->Disconnect());

  ret = amfi_service->SetDeveloperMode(mode);
  printf("ret: %d\n", ret);

  return ret;
}
