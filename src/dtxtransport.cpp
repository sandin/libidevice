#include "idevice/dtxtransport.h"

using namespace idevice;

bool DTXTransport::Connect() {
  return instrument_service_->Connect(device_) == InstrumentService::ResultCode::kOk;
}

bool DTXTransport::Disconnect() {
  if (IsConnected()) {
    instrument_service_->Disconnect();
  }
}

bool DTXTransport::IsConnected() const {
  return instrument_service_->IsConnected();
}
