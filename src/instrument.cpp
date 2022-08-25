#include "idevice/instrument.h"

#include <cstring>     // strcmp
#include <functional>  // std::bind, std::placeholders

#include "idevice/macro_def.h"

using namespace idevice;

constexpr char kInstrumentServiceName[] = "com.apple.instruments.remoteserver";
constexpr char kInstrumentSecureServiceName[] =
    "com.apple.instruments.remoteserver.DVTSecureSocketProxy";

IService::Result InstrumentService::NewClient(idevice_t device, lockdownd_service_descriptor_t service,
                                    InstrumentClient* client) {
  service_client_t service_client = nullptr;
  if (!device || !service || service->port == 0 || !client || *client) {
    return ResultCode::kInvalidArg;
  }
  service_error_t err = service_client_new(device, service, &service_client);
  if (err != kResultOk) {
    return err;
  }

  if (service->identifier && (strcmp(service->identifier, kInstrumentSecureServiceName) != 0)) {
    service_disable_bypass_ssl(service_client, 1);
  }

  *client = service_client;
  return ResultCode::kOk;
}

IService::Result InstrumentService::Connect(idevice_t device) {
  device_ = device;
  int32_t err = 0;
  
  // with ssl
  service_error_t ret = service_client_factory_start_service(
      device_, kInstrumentSecureServiceName, (void**)&client_, kClientLabel,
      SERVICE_CONSTRUCTOR(InstrumentService::NewClient), &err);
  IDEVICE_LOG_D("service_client_factory_start_service, name=%s, ret=%d\n", kInstrumentSecureServiceName, ret);
  if (ret == kResultOk) {
    return ResultCode::kOk;
  }
  
  // fallback, without ssl
  ret = service_client_factory_start_service(
      device_, kInstrumentServiceName, (void**)&client_, kClientLabel,
      SERVICE_CONSTRUCTOR(InstrumentService::NewClient), &err);
  IDEVICE_LOG_D("service_client_factory_start_service, name=%s, ret=%d\n", kInstrumentSecureServiceName, ret);
  return ret;
}

IService::Result InstrumentService::Disconnect() {
  device_ = nullptr;
  if (client_ != nullptr) {
    service_client_free(client_);
    client_ = nullptr;
  }
  return ResultCode::kOk;
}

IService::Result InstrumentService::Send(const char* data, uint32_t size, uint32_t* sent) {
  return service_send(client_, data, size, sent);
}

IService::Result InstrumentService::Receive(char* buffer, uint32_t size, uint32_t* received) {
  return service_receive(client_, buffer, size, received);
}

IService::Result InstrumentService::ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received) {
  return service_receive_with_timeout(client_, buffer, size, received, timeout);
}
