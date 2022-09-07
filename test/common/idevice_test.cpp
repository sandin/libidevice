#include "idevice/common/idevice.h"

#include <gtest/gtest.h>

#include <fstream>  // std::ofstream

#include "idevice/instrument/dtxchannel.h"
#include "idevice/instrument/dtxconnection.h"
#include "idevice/instrument/dtxtransport.h"
#include "libimobiledevice/libimobiledevice.h"
#include "nskeyedarchiver/scope.hpp"

using namespace idevice;

#define GET_FIRST_DEVICE()                                                  \
  ({                                                                        \
    char** devices;                                                         \
    int count = 0;                                                          \
    int ret = idevice_get_device_list(&devices, &count);                    \
    ASSERT_EQ(ret, IDEVICE_E_SUCCESS);                                      \
    if (count == 0) {                                                       \
      printf("no connected device!\n");                                     \
      ASSERT_TRUE(false);                                                   \
    }                                                                       \
    for (int i = 0; i < count; ++i) {                                       \
      char* device_udid = devices[i];                                       \
      printf("%d UDID: %s\n", i, device_udid);                              \
    }                                                                       \
    std::string udid = devices[0];                                          \
    idevice_device_list_free(devices);                                      \
                                                                            \
    idevice_t device = nullptr;                                             \
    idevice_new_with_options(&device, udid.c_str(), IDEVICE_LOOKUP_USBMUX); \
    if (device == nullptr) {                                                \
      printf("Can not create a new device(udid: %s).\n", udid.c_str());     \
      ASSERT_TRUE(false);                                                   \
    }                                                                       \
                                                                            \
    device;                                                                 \
  })

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

TEST(idevice, Test) {
  /* TODO: move this to the tools directory
  idevice_t device = GET_FIRST_DEVICE();
  defer(device, idevice_free(device));

#ifdef DEBUG_DTXTRANSPORT
  IDTXTransport* transport = new DebugProxyTransport(
      new DTXTransport(device), "idevice_transmit_outfile.bin", "idevice_received_outfile.bin");
#else
  IDTXTransport* transport = new DTXTransport(device);
#endif
  defer(transport, delete transport);

  DTXConnection* connection = new DTXConnection(transport);
  defer(connection, {
    connection->Disconnect();
    delete connection;
  });
  if (!connection->Connect()) {
    ASSERT_TRUE(false);
  }

  std::shared_ptr<DTXChannel> channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");

  std::shared_ptr<DTXMessage> message = DTXMessage::CreateWithSelector("runningProcesses");
  channel->SendMessageAsync(message, [&](auto msg) {
    printf("runningProcesses rseponse message:\n");
    msg->Dump();
    // TODO:
  });

  connection->DumpStat();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  DTXChannel* device_info_channel =
  connection.MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");

  std::shared_ptr<DTXMessage> message = std::make_shared<DTXMessage>("runningProcess", {0});
  auto callback = [=](auto resp) {};
  dtx_channel.SendMessageAsync(message, callback);

  connection.CannelChannel(device_info_channel);
  connection.Disconnect();
  free(device_info_channel);
  free(connection);
  */
}
