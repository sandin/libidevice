#include "instruments.hpp"

#include <string>
#include <thread>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "idevice/instrument/dtxchannel.h"
#include "idevice/instrument/dtxconnection.h"
#include "idevice/instrument/dtxtransport.h"
#include "idevice/instrument/kperf.h"
#include "libimobiledevice/libimobiledevice.h"
#include "nskeyedarchiver/scope.hpp"
#include "nskeyedarchiver/kavalue.hpp"
#include "nskeyedarchiver/kaarray.hpp"

using namespace idevice;

#define defer(label, exp) auto label##_grand = nskeyedarchiver::make_scope_exit([&]() { exp; });

#define NSMutableSet_t nskeyedarchiver::KAArray
#define NSMutableSet(...) nskeyedarchiver::KAArray("NSMutableSet", {"NSMutableSet", "NSSet", "NSObject"}, ##__VA_ARGS__)
#define NSSet_t nskeyedarchiver::KAArray
#define NSSet(...) nskeyedarchiver::KAArray("NSSet", {"NSSet", "NSObject"}, ##__VA_ARGS__)
#define NSValue(s) nskeyedarchiver::KAValue(s)

// TODO: delete it
struct idevice_private {
	char *udid;
	uint32_t mux_id;
	enum idevice_connection_type conn_type;
	void *conn_data;
	int version;
	int device_class;
};

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
  printf("coreprofilesessiontap:\n");
  
  /*
  // get traceCodesFile
  {
    auto device_info_channel =
        connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.deviceinfo");
    auto message = DTXMessage::CreateWithSelector("traceCodesFile");
    auto response = device_info_channel->SendMessageSync(message);
    printf("%s\n", response->PayloadObject()->ToJson().c_str());
    device_info_channel->Cancel();
  }
  */
  
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.coreprofilesessiontap");
  channel->SetMessageHandler([=](std::shared_ptr<DTXMessage> msg) {
    if (msg->PayloadObject()) {
      printf("%s\n", msg->PayloadObject()->ToJson().c_str());
    }
  });
  
  DTKTraceTapTriggerConfig trigger_config;
  trigger_config.SetKind(3);
  trigger_config.SetUUID("1DE83AEE-41CA-42AC-BAD9-19F504506EAB");
  trigger_config.SetKDebugFilter({ 630784000, 833617920, 830472456 });
  
  DTKTraceTapConfig config;
  config.SetRecodingPriority(10);
  config.SetPollingInterval(500);
  config.SetTriggerConfig(trigger_config);
  
  auto message1 = DTXMessage::CreateWithSelector("setConfig:");
  message1->AppendAuxiliary(config.ToKAValue());
  auto response1 = channel->SendMessageSync(message1);
  response1->Dump();
 
  auto message2 = DTXMessage::CreateWithSelector("start");
  auto response2 = channel->SendMessageSync(message2);
  response2->Dump();
 
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  return 0;
}

int graphics_opengl(DTXConnection* connection) {
  printf("graphics_opengl:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.graphics.opengl");
  channel->SetMessageHandler([=](std::shared_ptr<DTXMessage> msg) {
    if (msg->PayloadObject()) {
      printf("%s\n", msg->PayloadObject()->ToJson().c_str());
    }
  });
  
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
  
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  
  channel->Cancel();
  return 0;
}

int networking(DTXConnection* connection) {
  printf("networking:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.instruments.server.services.networking");
  channel->SetMessageHandler([=](std::shared_ptr<DTXMessage> msg) {
    if (msg->PayloadObject()) {
      printf("%s\n", msg->PayloadObject()->ToJson().c_str());
    }
  });
 
  auto message = DTXMessage::CreateWithSelector("startMonitoring");
  channel->SendMessageSync(message);
  
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  
  channel->Cancel();
  return 0;
}

int energy(DTXConnection* connection, uint64_t pid) {
  printf("Energy:\n");
  auto channel =
      connection->MakeChannelWithIdentifier("com.apple.xcode.debug-gauge-data-providers.Energy");

  channel->SetMessageHandler([=](std::shared_ptr<DTXMessage> msg) {
    if (msg->PayloadObject()) {
      printf("%s\n", msg->PayloadObject()->ToJson().c_str());
    }
  });
 
  {
    auto message = DTXMessage::CreateWithSelector("startSamplingForPIDs:");
    NSMutableSet_t pids = NSMutableSet({nskeyedarchiver::KAValue(pid)});
    message->AppendAuxiliary(NSValue(std::move(pids)));
    channel->SendMessageSync(message);
  }
  
  while (true) {
    auto message = DTXMessage::CreateWithSelector("sampleAttributes:forPIDs:");
    // attributes
    NSSet_t attributes = NSSet({
      NSValue("energy.cost"),
      NSValue("energy.CPU"),
      NSValue("energy.networking"),
      NSValue("energy.location"),
      NSValue("energy.GPU"),
      NSValue("energy.appstate"),
      NSValue("energy.overhead"),
    });
    message->AppendAuxiliary(NSValue(std::move(attributes)));
    // pids
    NSMutableSet_t pids = NSMutableSet({ NSValue(pid) });
    message->AppendAuxiliary(NSValue(std::move(pids)));
    
    channel->SendMessageSync(message);
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  
  channel->Cancel();
  return 0;
}

int idevice::tools::instruments_main(const idevice::tools::Args& args) {
  idevice_set_debug_level(1);

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

  // RSD for iOS 17
  std::string host = get_flag_as_str(args, "host", ""); // RSD handshake address
  int port = get_flag_as_int(args, "port", 0);          // RSD com.apple.mobile.lockdown.remote.trusted port 
  bool rsd = !host.empty() && port != 0;
  if (rsd) {
    struct sockaddr_in6* addr = (struct sockaddr_in6*)(malloc(sizeof(sockaddr_in6)));
    memset(addr, 0, sizeof(addr));
    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons(port);
    inet_pton(AF_INET6, host.c_str(), &addr->sin6_addr); 

    device->conn_data = addr; 
    device->conn_type = CONNECTION_NETWORK;
  }

  IDTXTransport* transport = new DTXTransport(device, rsd);
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
  std::this_thread::sleep_for(std::chrono::seconds(3));

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
  } else if (command == "networking") {
    ret = networking(connection);
  } else if (command == "energy") {
    uint64_t pid = idevice::tools::get_flag_as_int(args, "pid", 0);
    ret = energy(connection, pid);
  } else {
    printf("unknown command: %s\n", command.c_str());
  }
  
  //while (true) {
  //  std::this_thread::sleep_for(std::chrono::seconds(3));
  //}
  return ret;
}
