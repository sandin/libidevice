#ifndef IDEVICE_INSTRUMENT_H
#define IDEVICE_INSTRUMENT_H

#include "libimobiledevice/libimobiledevice.h"
#include "libimobiledevice/lockdown.h"
#include "libimobiledevice/service.h"

#include "idevice/idevice.h"
#include "idevice/iservice.h"

namespace idevice {

// Instrument Service
class InstrumentService : public IService {
public:
  enum ResultCode {
    kOk = kResultOk,
    kInvalidArg = -1,
    kPlistError = -2,
    kUsbmuxError = -3,
    kSSLError = -4,
    kReceiveTimeout = -5,
    kBadVersion = -6,
    kConnFailed = -7,
    kUnknownError = kResultUnknownError
  };
  
  using InstrumentClient = service_client_t;
  
  InstrumentService(idevice_t device) : device_(device) {}
  virtual ~InstrumentService() override {}
  
  virtual Result Start() override;
  virtual Result Stop() override;
  
  Result Send(const char* data, uint32_t size, uint32_t* sent);
  Result Receive(char* buffer, uint32_t size, uint32_t* received);
  Result ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received);
  
private:
  static Result NewClient(idevice_t device, lockdownd_service_descriptor_t service, InstrumentClient* client);
  
  idevice_t device_ = nullptr;
  InstrumentClient client_ = nullptr;
}; // class InstrumentService

}  // namespace idevice

#endif // IDEVICE_INSTRUMENT_H

