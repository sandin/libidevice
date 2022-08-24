#include "idevice/dtxtransport.h"

using namespace idevice;

bool DTXTransport::Connect() {
  InstrumentService::Result result = instrument_service_->Connect(device_);
  printf("Connect result: %d\n", result);
  return result == InstrumentService::ResultCode::kOk;
}

bool DTXTransport::Disconnect() {
  if (IsConnected()) {
    instrument_service_->Disconnect();
  }
}

bool DTXTransport::IsConnected() const {
  return instrument_service_->IsConnected();
}

bool DTXTransport::Send(const char* data, uint32_t size, uint32_t* sent) {
  return instrument_service_->Send(data, size, sent) == InstrumentService::ResultCode::kOk;
}

bool DTXTransport::Receive(char* buffer, uint32_t size, uint32_t* received) {
  return instrument_service_->Receive(buffer, size, received) == InstrumentService::ResultCode::kOk;
}

bool DTXTransport::ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received) {
  InstrumentService::Result result = instrument_service_->ReceiveWithTimeout(buffer, size, timeout, received);
  //printf("ReceiveWithTimeout result: %d\n", result);
  return result == InstrumentService::ResultCode::kOk || result == InstrumentService::ResultCode::kTimeout;
}
