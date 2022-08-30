#ifndef IDEVICE_INSTRUMENT_H
#define IDEVICE_INSTRUMENT_H

#include "idevice/iservice.h"
#include "idevice/idevice.h"
#include "libimobiledevice/libimobiledevice.h"
#include "libimobiledevice/lockdown.h"
#include "libimobiledevice/service.h"

namespace idevice {

// Instrument Service
class InstrumentService : public IService {
 public:
  enum ResultCode {
    kOk = kResultOk,
    kInvalidArg = -1,
    kMuxError = -3,
    kSSLError = -4,
    kStartServiceError = -5,
    kNotEnoughData = -6,
    kTimeout = -7,
    kUnknownError = kResultUnknownError
  };

  using InstrumentClient = service_client_t;

  InstrumentService() {}
  virtual ~InstrumentService() override {}

  virtual Result Connect(idevice_t device) override;
  virtual Result Disconnect() override;

  Result Send(const char* data, uint32_t size, uint32_t* sent);
  Result Receive(char* buffer, uint32_t size, uint32_t* received);
  Result ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received);

  bool IsConnected() const { return client_ != nullptr; }

 private:
  static Result NewClient(idevice_t device, lockdownd_service_descriptor_t service,
                          InstrumentClient* client);

  idevice_t device_ = nullptr;
  InstrumentClient client_ = nullptr;
};  // class InstrumentService

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_H
