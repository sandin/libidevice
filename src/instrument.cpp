#include "idevice/instrument.h"

#include <cstring>     // strcmp
#include <functional>  // std::bind, std::placeholders

using namespace idevice;

static const char kInstrumentServiceName[] = "com.apple.instruments.remoteserver";
static const char kInstrumentSecureServiceName[] =
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

IService::Result InstrumentService::Start() {
  int32_t err = 0;
  
  // with ssl
  service_error_t ret = service_client_factory_start_service(
      device_, kInstrumentSecureServiceName, (void**)&client_, kClientLabel,
      SERVICE_CONSTRUCTOR(InstrumentService::NewClient), &err);
  if (ret == kResultOk) {
    return ResultCode::kOk;
  }
  
  // fallback, without ssl
  ret = service_client_factory_start_service(
      device_, kInstrumentServiceName, (void**)&client_, kClientLabel,
      SERVICE_CONSTRUCTOR(InstrumentService::NewClient), &err);
  
  return ret;
}

IService::Result InstrumentService::Stop() {
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
