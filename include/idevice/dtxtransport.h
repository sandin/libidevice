#ifndef IDEVICE_DTXTRANSPORT_H
#define IDEVICE_DTXTRANSPORT_H

#include <cstdint>
#include <cstdio>
#include <cstring> // memcpy
#include <algorithm> // std::min

#include "idevice/idevice.h" // hexdump
#include "idevice/instrument.h"

namespace idevice {

// Interface Service
class IDTXTransport {
 public:
  virtual ~IDTXTransport() {}
  
  virtual bool Connect() = 0;
  virtual bool Disconnect() = 0;
  virtual bool IsConnected() const  = 0;
  
  virtual bool Send(const char* data, uint32_t size, uint32_t* sent) = 0;
  virtual bool Receive(char* buffer, uint32_t size, uint32_t* received) = 0;
  virtual bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received) = 0;
};

/**
 * DTXTransport
 */
class DTXTransport : public IDTXTransport {
 public:
  DTXTransport(idevice_t device) : device_(device) {
    instrument_service_ = new InstrumentService();
  }
  
  virtual ~DTXTransport() {
    if (instrument_service_ != nullptr) {
      delete instrument_service_;
    }
  }
  
  virtual bool Connect() override;
  virtual bool Disconnect() override;
  virtual bool IsConnected() const override;
  
  virtual bool Send(const char* data, uint32_t size, uint32_t* sent) override;
  virtual bool Receive(char* buffer, uint32_t size, uint32_t* received) override;
  virtual bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received) override;

 private:
  idevice_t device_ = nullptr;
  InstrumentService* instrument_service_ = nullptr;

}; // class DTXTransport

/**
 * BufferedDTXTransport
 */
class BufferedDTXTransport {
 public:
  BufferedDTXTransport(IDTXTransport* transport, char* buffer, size_t buffer_size)
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
    bool ret = transport_->Send(buffer_, buffer_used_, &actual_size);
    buffer_used_ = 0;
    return ret;
  }

 private:
  IDTXTransport* transport_ = nullptr;
  char* buffer_ = nullptr;
  size_t buffer_size_ = 0;
  size_t buffer_used_ = 0;
}; // BufferedDTXTransport

}  // namespace idevice

#endif // IDEVICE_DTXTRANSPORT_H
