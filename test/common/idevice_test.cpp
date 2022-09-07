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

// when this flag is turned on, we will write all sent and received messages
// to two binary files to make it easy to examine them.
// By use 010Editor with the template named `DTXMessage.bt` located in the `tools` directory.
#define DEBUG_DTXTRANSPORT
#ifdef DEBUG_DTXTRANSPORT
class DebugProxyTransport : public IDTXTransport {
 public:
  DebugProxyTransport(IDTXTransport* proxy, const char* transmit_outfilename,
                      const char* received_outfilename)
      : proxy_(proxy),
        transmit_outfile_(transmit_outfilename, std::ofstream::binary),
        received_outfile_(received_outfilename, std::ofstream::binary) {}
  virtual ~DebugProxyTransport() {
    if (proxy_ != nullptr) {
      delete proxy_;
    }
  }

  virtual bool Connect() override { return proxy_->Connect(); }
  virtual bool Disconnect() override { return proxy_->Disconnect(); }
  virtual bool IsConnected() const override { return proxy_->IsConnected(); }
  virtual bool Send(const char* data, uint32_t size, uint32_t* sent) override {
    bool ret = proxy_->Send(data, size, sent);
    if (ret) {
      transmit_outfile_.write(data, *sent);
      transmit_outfile_.flush();
    }
    return ret;
  }
  virtual bool Receive(char* buffer, uint32_t size, uint32_t* received) override {
    bool ret = proxy_->Receive(buffer, size, received);
    if (ret) {
      received_outfile_.write(buffer, *received);
      received_outfile_.flush();
    }
    return ret;
  }
  virtual bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout,
                                  uint32_t* received) override {
    bool ret = proxy_->ReceiveWithTimeout(buffer, size, timeout, received);
    if (ret) {
      received_outfile_.write(buffer, *received);
      received_outfile_.flush();
    }
    return ret;
  }

 private:
  IDTXTransport* proxy_;
  std::ofstream transmit_outfile_;
  std::ofstream received_outfile_;
};
#endif  // DEBUG_DTXTRANSPORT

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
