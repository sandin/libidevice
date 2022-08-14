#ifndef IDEVICE_DTXTRANSPORT_H
#define IDEVICE_DTXTRANSPORT_H

#include <cstdint>
#include <cstdio>

#include "idevice/instrument.h"

namespace idevice {

class DTXTransport {
 public:
  DTXTransport(idevice_t device) : device_(device) {
    instrument_service_ = new InstrumentService();
  }
  
  virtual ~DTXTransport() {
    if (instrument_service_ != nullptr) {
      delete instrument_service_;
    }
  }
  
  bool Connect();
  bool Disconnect();
  bool IsConnected() const;
  
  bool Send(const char* data, uint32_t size, uint32_t* sent);
  bool Receive(char* buffer, uint32_t size, uint32_t* received);
  bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received);

 private:
  idevice_t device_ = nullptr;
  InstrumentService* instrument_service_ = nullptr;

}; // class DTXService

}  // namespace idevice

#endif // IDEVICE_DTXTRANSPORT_H
