#ifndef IDEVICE_SERVICE_ISERVICE_H
#define IDEVICE_SERVICE_ISERVICE_H

#include <cstdint>

#include "libimobiledevice/libimobiledevice.h"

namespace idevice {

// Interface Service
class IService {
 public:
  using Result = uint32_t;

  virtual ~IService() {}

  /**
   * Connect to the server
   * 
   * @param device the target device
   * @return Result result code
   */
  virtual Result Connect(idevice_t device) = 0;

  /**
   * Disconnect from the server 
   * 
   * @return Result result code
   */
  virtual Result Disconnect() = 0;
};

}  // namespace idevice

#endif  // IDEVICE_SERVICE_ISERVICE_H
