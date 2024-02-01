#ifndef IDEVICE_INSTRUMENT_INSTRUMENT_H
#define IDEVICE_INSTRUMENT_INSTRUMENT_H

#include "idevice/service/iservice.h"
#include "idevice/common/idevice.h"
#include "libimobiledevice/libimobiledevice.h"
#include "libimobiledevice/lockdown.h"
#include "libimobiledevice/service.h"

namespace idevice {

/**
 * The client for communication with instrument service 
 */
class InstrumentService : public IService {
 public:
  enum ResultCode {
    kOk = kResultOk,
    kInvalidArg = -1,
    kMuxError = -3,
    kSSLError = -4,
    kStartServiceError = -5,
    kNotEnoughData = -6,
    kTimeout = -7,
    kUnknownError = kResultUnknownError
  };

  using InstrumentClient = service_client_t;

  /**
   * Constructor
   */
  InstrumentService(bool rsd) {
    rsd_ = rsd;
  }

  /**
   * Destructor
   */
  virtual ~InstrumentService() override {}

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

  /**
   * Check whether it's connected or not
   * 
   * @return connected or not 
   */
  bool IsConnected() const { return client_ != nullptr; }

  /**
   * Write data to the server 
   * 
   * @param data the buffer
   * @param size size of the buffer
   * @param sent actual sent size
   * @return succeed or fail
   */
  Result Send(const char* data, uint32_t size, uint32_t* sent);

  /**
   * Read data from the server 
   * 
   * @param buffer the buffer
   * @param size size of the buffer
   * @param received actual received size
   * @return succeed or fail
   */
  Result Receive(char* buffer, uint32_t size, uint32_t* received);

  /**
   * Read data from the server with a timeout 
   * 
   * @param buffer the buffer
   * @param size  size of the buffer
   * @param timeout timeout in milliseconds
   * @param received actual received size
   * @return succeed or fail
   */
  Result ReceiveWithTimeout(char* buffer, uint32_t size, uint32_t timeout, uint32_t* received);

 private:
  static Result NewClient(idevice_t device, lockdownd_service_descriptor_t service,
                          InstrumentClient* client);

  idevice_t device_ = nullptr;
  InstrumentClient client_ = nullptr;
  bool rsd_ = false;
};  // class InstrumentService

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_INSTRUMENT_H
