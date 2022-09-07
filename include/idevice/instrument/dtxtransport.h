#ifndef IDEVICE_INSTRUMENT_DTXTRANSPORT_H
#define IDEVICE_INSTRUMENT_DTXTRANSPORT_H

#include <algorithm>  // std::min
#include <cstdint>
#include <cstdio>
#include <cstring>  // memcpy
#include <fstream>  // std::ofstream

#include "idevice/common/idevice.h"  // hexdump
#include "idevice/instrument/instrument.h"

namespace idevice {

/**
 * Interface of transport that can send and receive DTXMessage.
 */
class IDTXTransport {
 public:
  virtual ~IDTXTransport() {}

  /**
   * Connect to the service 
   * 
   * @return succeed or fail 
   */
  virtual bool Connect() = 0;

  /**
   * Disconnect from the service
   * 
   * @return succeed or fail  
   */
  virtual bool Disconnect() = 0;

  /**
   * Check whether it's connected or not
   * 
   * @return connected or not 
   */
  virtual bool IsConnected() const = 0;

  /**
   * Write data to the server 
   * 
   * @param data the buffer
   * @param size size of the buffer
   * @param sent actual sent size
   * @return succeed or fail
   */
  virtual bool Send(const char* data, uint32_t size, uint32_t* sent) = 0;
  
  /**
   * Read data from the server 
   * 
   * @param buffer the buffer
   * @param size size of the buffer
   * @param received actual received size
   * @return succeed or fail
   */
  virtual bool Receive(char* buffer, uint32_t size, uint32_t* received) = 0;

  /**
   * Read data from the server with a timeout 
   * 
   * @param buffer the buffer
   * @param size  size of the buffer
   * @param timeout timeout in milliseconds
   * @param received actual received size
   * @return succeed or fail
   */
  virtual bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout,
                                  uint32_t* received) = 0;
};

/**
 * DTXTransport, the default transport responsible for receiving and sending data.
 */
class DTXTransport : public IDTXTransport {
 public:
  /**
   * Constructor
   * 
   * @param device the target of transport
   */
  DTXTransport(idevice_t device) : device_(device) {
    instrument_service_ = new InstrumentService();
  }

  /**
   * Destructor
   * 
   */
  virtual ~DTXTransport() {
    if (instrument_service_ != nullptr) {
      delete instrument_service_;
    }
  }

  /**
   * Connect to the service 
   * 
   * @return succeed or fail 
   */
  virtual bool Connect() override;

  /**
   * Disconnect from the service
   * 
   * @return succeed or fail  
   */
  virtual bool Disconnect() override;

  /**
   * Check whether it's connected or not
   * 
   * @return connected or not 
   */
  virtual bool IsConnected() const override;

  /**
   * Write data to the server 
   * 
   * @param data the buffer
   * @param size size of the buffer
   * @param sent actual sent size
   * @return succeed or fail
   */
  virtual bool Send(const char* data, uint32_t size, uint32_t* sent) override;

  /**
   * Read data from the server 
   * 
   * @param buffer the buffer
   * @param size size of the buffer
   * @param received actual received size
   * @return succeed or fail
   */
  virtual bool Receive(char* buffer, uint32_t size, uint32_t* received) override;

  /**
   * Read data from the server with a timeout 
   * 
   * @param buffer the buffer
   * @param size  size of the buffer
   * @param timeout timeout in milliseconds
   * @param received actual received size
   * @return succeed or fail
   */
  virtual bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout,
                                  uint32_t* received) override;

 private:
  idevice_t device_ = nullptr;
  InstrumentService* instrument_service_ = nullptr;

};  // class DTXTransport

/**
 * BufferedDTXTransport, a transport with a fixed size buffer to improve IO efficiency.
 */
class BufferedDTXTransport {
 public:

  /**
   * Constructor
   * 
   * @param transport the real transport 
   * @param buffer the buffer
   * @param buffer_size size of the buffer
   */
  BufferedDTXTransport(IDTXTransport* transport, char* buffer, size_t buffer_size)
      : transport_(transport), buffer_(buffer), buffer_size_(buffer_size) {}

  /**
   * Destructor
   * 
   */
  ~BufferedDTXTransport() {
    if (buffer_used_ >= buffer_size_) {
      Flush();
    }
  }

  /**
   * Write data to the server 
   * 
   * @param data the buffer
   * @param size size of the buffer
   * @return succeed or fail
   */
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

  /**
   * Flush all data in the buffer to the server 
   * 
   * @return succeed or fail
   */
  bool Flush() {
    if (buffer_used_ == 0) {
      return;  // nothing to do
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
};  // BufferedDTXTransport

// when this flag is turned on, we will write all sent and received messages
// to two binary files to make it easy to examine them.
// By use 010Editor with the template named `DTXMessage.bt` located in the `tools` directory.
#define DEBUG_DTXTRANSPORT
#ifdef DEBUG_DTXTRANSPORT
class DebugProxyTransport : public IDTXTransport {
 public:
  DebugProxyTransport(IDTXTransport* proxy, const char* transmit_outfilename,
                      const char* received_outfilename)
      : proxy_(proxy),
        transmit_outfile_(transmit_outfilename, std::ofstream::binary),
        received_outfile_(received_outfilename, std::ofstream::binary) {}
  virtual ~DebugProxyTransport() {
    if (proxy_ != nullptr) {
      delete proxy_;
    }
  }

  virtual bool Connect() override { return proxy_->Connect(); }
  virtual bool Disconnect() override { return proxy_->Disconnect(); }
  virtual bool IsConnected() const override { return proxy_->IsConnected(); }
  virtual bool Send(const char* data, uint32_t size, uint32_t* sent) override {
    bool ret = proxy_->Send(data, size, sent);
    if (ret) {
      transmit_outfile_.write(data, *sent);
      transmit_outfile_.flush();
    }
    return ret;
  }
  virtual bool Receive(char* buffer, uint32_t size, uint32_t* received) override {
    bool ret = proxy_->Receive(buffer, size, received);
    if (ret) {
      received_outfile_.write(buffer, *received);
      received_outfile_.flush();
    }
    return ret;
  }
  virtual bool ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout,
                                  uint32_t* received) override {
    bool ret = proxy_->ReceiveWithTimeout(buffer, size, timeout, received);
    if (ret) {
      received_outfile_.write(buffer, *received);
      received_outfile_.flush();
    }
    return ret;
  }

 private:
  IDTXTransport* proxy_;
  std::ofstream transmit_outfile_;
  std::ofstream received_outfile_;
};
#endif  // DEBUG_DTXTRANSPORT

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_DTXTRANSPORT_H
