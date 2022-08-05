#ifndef IDEVICE_DTXTRANSPORT_H
#define IDEVICE_DTXTRANSPORT_H

#include <cstdint>
#include <cstdio>

#include "idevice/instrument.h"

namespace idevice {

class DTXTransport {
 public:
  DTXTransport() {
    transport_ = new InstrumentService();
  }

  virtual ~DTXTransport() {
    if (transport_ != nullptr) {
      if (transport_->IsConnected()) {
        transport_->Disconnect();
      }
      delete transport_;
    }
  }

 private:
  InstrumentService* transport_ = nullptr;

}; // class DTXService

}  // namespace idevice

#endif // IDEVICE_DTXTRANSPORT_H
