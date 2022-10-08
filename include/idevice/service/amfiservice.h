#ifndef IDEVICE_SERVICE_AMFI_H
#define IDEVICE_SERVICE_AMFI_H

#include <string>

#include "idevice/common/idevice.h"
#include "idevice/service/iservice.h"
#include "libimobiledevice/libimobiledevice.h"
#include "libimobiledevice/lockdown.h"
#include "libimobiledevice/property_list_service.h"
#include "libimobiledevice/service.h"

namespace idevice {

/**
 * AMFI Lockdown
 */
class AMFIService : public IService {
 public:
  enum ResultCode { kOk = kResultOk, kInvalidArg = -1, kUnknownError = kResultUnknownError };

  using AMFIClient = property_list_service_client_t;

  /**
   * Constructor
   */
  AMFIService() {}

  /**
   * Destructor
   */
  virtual ~AMFIService() override {}

  /**
   * Connect to the server
   *
   * @param device the target device
   * @return Result result code
   */
  virtual Result Connect(idevice_t device) override;

  /**
   * Disconnect from the server
   *
   * @return Result result code
   */
  virtual Result Disconnect() override;

  Result SetDeveloperMode(int mode);

 private:
  static Result NewClient(idevice_t device, lockdownd_service_descriptor_t service,
                          AMFIClient* client);

  AMFIClient client_ = nullptr;
};  // class LockdownService

}  // namespace idevice

#endif  // IDEVICE_SERVICE_AMFI_H