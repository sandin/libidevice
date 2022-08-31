#include "instruments.hpp"

#include <string>
#include <thread>

#include "idevice/dtxchannel.h"
#include "idevice/dtxconnection.h"
#include "idevice/dtxtransport.h"
#include "libimobiledevice/libimobiledevice.h"
#include "nskeyedarchiver/scope.hpp"

using namespace idevice;

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

int running_processes(DTXConnection* connection) {
  printf("runningProcesses:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
  auto message = DTXMessage::CreateWithSelector("runningProcesses");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  return 0;
}

int request_device_gpu_info(DTXConnection* connection) {
  printf("requestDeviceGPUInfo:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.gpu");
  auto message = DTXMessage::CreateWithSelector("requestDeviceGPUInfo");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  return 0;
}

int idevice::tools::instruments_main(const idevice::tools::Args& args) {
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

  IDTXTransport* transport = new DTXTransport(device);
  defer(transport, delete transport);

  DTXConnection* connection = new DTXConnection(transport);
  defer(connection, {
    connection->Disconnect();
    delete connection;
  });
  if (!connection->Connect()) {
    printf("Can not connect to the device(uuid: %s).\n", udid.c_str());
    return -1;
  }
  std::this_thread::sleep_for(std::chrono::seconds(3)); // TODO: delete me

  std::string command = args.first.at(0);
  if (command == "running_processes") {
    return running_processes(connection);
  } else if (command == "request_device_gpu_info") {
    return request_device_gpu_info(connection);
  } else {
    printf("unknown command: %s\n", command.c_str());
  }
}
