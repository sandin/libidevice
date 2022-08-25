#ifndef IDEVICE_DTXTRANSPORT_H
#define IDEVICE_DTXTRANSPORT_H

#include <cstdint>
#include <cstdio>
#include <cstring> // memcpy
#include <algorithm> // std::min

#include "idevice/macro_def.h"
#include "idevice/idevice.h" // hexdump
#include "idevice/instrument.h"

namespace idevice {

/**
 * DTXTransport
 */
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

}; // class DTXTransport

/**
 * BufferedDTXTransport
 */
class BufferedDTXTransport {
 public:
  BufferedDTXTransport(DTXTransport* transport, char* buffer, size_t buffer_size)
    : transport_(transport), buffer_(buffer), buffer_size_(buffer_size) {}
  
  ~BufferedDTXTransport() {
    if (buffer_used_ >= buffer_size_) {
      Flush();
    }
  }
  
  bool Send(const char* data, size_t size) {
    if (size >= buffer_size_) {
      if (!Flush()) {
        return false;
      }
#if IDEVICE_DEBUG
      IDEVICE_LOG_D("Send\n");
      hexdump((void*)(data), size, 0);
#endif
      uint32_t actual_size = 0;
      return transport_->Send(data, size, &actual_size);
    }
    
    size_t offset = 0;
    while (offset < size) {
      size_t len = std::min(buffer_size_ - buffer_used_, size - offset);
      memcpy(buffer_ + buffer_used_, data + offset, len);
      offset += len;
      buffer_used_ += len;
      if (buffer_used_ >= buffer_size_) {
        if (!Flush()) {
          return false;
        }
      }
    }
    return true;
  }
  
  bool Flush() {
    if (buffer_used_ == 0) {
      return; // nothing to do
    }
    uint32_t actual_size = 0;
#if IDEVICE_DEBUG
    IDEVICE_LOG_D("Flush\n");
    hexdump(buffer_, buffer_used_, 0);
#endif
    bool ret = transport_->Send(buffer_, buffer_used_, &actual_size);
    buffer_used_ = 0;
    return ret;
  }

 private:
  DTXTransport* transport_ = nullptr;
  char* buffer_ = nullptr;
  size_t buffer_size_ = 0;
  size_t buffer_used_ = 0;
}; // BufferedDTXTransport

}  // namespace idevice

#include "idevice/macro_undef.h"

#endif // IDEVICE_DTXTRANSPORT_H
