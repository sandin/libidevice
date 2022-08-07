#include "idevice/idevice.h"

#include <gtest/gtest.h>

#include "idevice/dtxchannel.h"

using namespace idevice;

TEST(idevice, Test) {
  
  /*
  device_t device;
  DTXTransport* transport = new DTXTransport(device);
  DTXConnection* connection = new DTXConnection(transport);
  if (!connection.Connect(device)) {
   ASSERT_TRUE(false);
  }
  DTXChannel* device_info_channel = connection.MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
   
  std::shared_ptr<DTXMessage> message = std::make_shared<DTXMessage>("runningProcess", {0});
  auto callback = [=](auto resp) {};
  dtx_channel.SendMessageAsync(message, callback);
   
  connection.CannelChannel(device_info_channel);
  connection.Disconnect();
  free(device_info_channel);
  free(connection);
  free(transport);
  */
  
  ASSERT_TRUE(true);
}
