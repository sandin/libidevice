#include "instruments.hpp"

#include <string>

#include "idevice/dtxchannel.h"
#include "idevice/dtxconnection.h"
#include "idevice/dtxtransport.h"
#include "libimobiledevice/libimobiledevice.h"
#include "nskeyedarchiver/scope.hpp"

using namespace idevice;

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

int running_processes(DTXConnection* connection) {
  printf("runningProcesses:\n");
  std::shared_ptr<DTXChannel> channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");

  std::shared_ptr<DTXMessage> message = DTXMessage::CreateWithSelector("runningProcesses");
  std::shared_ptr<DTXMessage> response = channel->SendMessageSync(message, -1);
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
  defer(transport, free(transport));

  DTXConnection* connection = new DTXConnection(transport);
  defer(connection, {
    connection->Disconnect();
    free(connection);
  });
  if (!connection->Connect()) {
    printf("Can not connect to the device(uuid: %s).\n", udid.c_str());
    return -1;
  }

  std::string command = args.first.at(0);
  if (command == "runningProcesses") {
    return running_processes(connection);
  } else {
    printf("unknown command: %s\n", command.c_str());
  }
}
