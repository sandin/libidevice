#ifndef IDEVICE_ISERVICE_H
#define IDEVICE_ISERVICE_H

#include <cstdint>
#include <cstdio>

namespace idevice {

// Interface Service
class IService {
 public:
  using Result = uint32_t;
  
  virtual ~IService() {}
  
  virtual Result Start() = 0;
  virtual Result Stop() = 0;
};

}  // namespace idevice

#endif // IDEVICE_ISERVICE_H
