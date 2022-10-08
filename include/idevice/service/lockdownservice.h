#ifndef IDEVICE_SERVICE_LOCKDOWN_H
#define IDEVICE_SERVICE_LOCKDOWN_H

#include <string>

#include "idevice/common/idevice.h"
#include "idevice/service/iservice.h"
#include "libimobiledevice/libimobiledevice.h"
#include "libimobiledevice/lockdown.h"
#include "libimobiledevice/service.h"

namespace idevice {

/**
 * Lockdown
 */
class LockdownService : public IService {
 public:
  /**
   * Constructor
   */
  LockdownService() {}

  /**
   * Destructor
   */
  virtual ~LockdownService() override {}

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

  std::string GetValue(const char* domain, const char* key, const std::string def_val);

 private:
  idevice_t device_ = nullptr;
  lockdownd_client_t client_ = nullptr;
};  // class LockdownService

}  // namespace idevice

#endif  // IDEVICE_SERVICE_LOCKDOWN_H