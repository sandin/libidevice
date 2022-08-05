#ifndef IDEVICE_ISERVICE_H
#define IDEVICE_ISERVICE_H

#include <cstdint>

#include "idevice/idevice.h"

namespace idevice {

// Interface Service
class IService {
 public:
  using Result = uint32_t;
  
  virtual ~IService() {}
  
  virtual Result Connect(idevice_t device) = 0;
  virtual Result Disconnect() = 0;
};

}  // namespace idevice

#endif // IDEVICE_ISERVICE_H
