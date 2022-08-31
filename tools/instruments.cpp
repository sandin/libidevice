#include "instruments.hpp"

#include <string>
#include <thread>

#include "idevice/dtxchannel.h"
#include "idevice/dtxconnection.h"
#include "idevice/dtxtransport.h"
#include "libimobiledevice/libimobiledevice.h"
#include "nskeyedarchiver/scope.hpp"
#include "nskeyedarchiver/kavalue.hpp"

using namespace idevice;

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

int running_processes(DTXConnection* connection) {
  printf("runningProcesses:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
  auto message = DTXMessage::CreateWithSelector("runningProcesses");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  channel->Cancel();
  return 0;
}

int hardware_information(DTXConnection* connection) {
  printf("hardwareInformation:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
  auto message = DTXMessage::CreateWithSelector("hardwareInformation");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  channel->Cancel();
  return 0;
}

int network_information(DTXConnection* connection) {
  printf("networkInformation:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
  auto message = DTXMessage::CreateWithSelector("networkInformation");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  channel->Cancel();
  return 0;
}

int mach_time_info(DTXConnection* connection) {
  printf("machTimeInfo:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
  auto message = DTXMessage::CreateWithSelector("machTimeInfo");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  channel->Cancel();
  return 0;
}

int execname_for_pid(DTXConnection* connection, uint64_t pid) {
  printf("execnameForPid:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
  auto message = DTXMessage::CreateWithSelector("execnameForPid:");
  message->AppendAuxiliary(nskeyedarchiver::KAValue(pid));
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  channel->Cancel();
  return 0;
}

int request_device_gpu_info(DTXConnection* connection) {
  printf("requestDeviceGPUInfo:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.gpu");
  auto message = DTXMessage::CreateWithSelector("requestDeviceGPUInfo");
  auto response = channel->SendMessageSync(message);
  printf("%s\n", response->PayloadObject()->ToJson().c_str());
  channel->Cancel();
  return 0;
}

int core_profile_session_tap(DTXConnection* connection) {
/*
  printf("coreprofilesessiontap:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.coreprofilesessiontap");
  TODO: setConfigs:
  auto message1 = DTXMessage::CreateWithSelector("setConfigs:");
  auto response1 = channel->SendMessageSync(message);
  printf("%s\n", response1->PayloadObject()->ToJson().c_str());
 
  auto message2 = DTXMessage::CreateWithSelector("start");
  auto response2 = channel->SendMessageSync(message);
  printf("%s\n", response2->PayloadObject()->ToJson().c_str());
 
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
 
  channel->Cancel();
*/
  printf("Unsupported method.\n");
  return 0;
}

int graphics_opengl(DTXConnection* connection) {
  printf("graphics_opengl:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.graphics.opengl");
  
  {
    auto message = DTXMessage::CreateWithSelector("setSamplingRate:");
    message->AppendAuxiliary(nskeyedarchiver::KAValue(10));
    channel->SendMessageSync(message);
  }
  {
    auto message = DTXMessage::CreateWithSelector("startSamplingAtTimeInterval:processIdentifier:");
    message->AppendAuxiliary(nskeyedarchiver::KAValue(0));
    message->AppendAuxiliary(nskeyedarchiver::KAValue(0));
    channel->SendMessageSync(message);
  }
  
  channel->SetMessageHandler([=](std::shared_ptr<DTXMessage> msg) {
    if (msg->PayloadObject()) {
      printf("%s\n", msg->PayloadObject()->ToJson().c_str());
    }
  });
  
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  
  channel->Cancel();
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
  //std::this_thread::sleep_for(std::chrono::seconds(3));

  std::string command = args.first.at(0);
  ret = 0;
  if (command == "running_processes") {
    ret = running_processes(connection);
  } else if (command == "request_device_gpu_info") {
    ret = request_device_gpu_info(connection);
  } else if (command == "hardware_information") {
    ret = hardware_information(connection);
  } else if (command == "network_information") {
    ret = network_information(connection);
  } else if (command == "mach_time_info") {
    ret = mach_time_info(connection);
  } else if (command == "execname_for_pid") {
    uint64_t pid = idevice::tools::get_flag_as_int(args, "pid", 0);
    ret = execname_for_pid(connection, pid);
  } else if (command == "core_profile_session_tap") {
    ret = core_profile_session_tap(connection);
  } else if (command == "graphics_opengl") {
    ret = graphics_opengl(connection);
  } else {
    printf("unknown command: %s\n", command.c_str());
  }
  
  //while (true) {
  //  std::this_thread::sleep_for(std::chrono::seconds(3));
  //}
  return ret;
}
