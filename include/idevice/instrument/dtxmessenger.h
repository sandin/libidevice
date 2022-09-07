#ifndef IDEVICE_INSTRUMENT_DTXMESSENGER_H
#define IDEVICE_INSTRUMENT_DTXMESSENGER_H

#include <functional>

namespace idevice {

class DTXMessage;
class DTXChannel;

/**
 * Interface of messenger that can send and receive DTXMessage.
 * The known inherited classes are DTXConnection and DTXChannel.
 */
class DTXMessenger {
 public:
  /**
   * Callback for response
   */
  using ReplyHandler = std::function<void(std::shared_ptr<DTXMessage>)>;

  virtual ~DTXMessenger() {}

  /**
   * Send message synchronously
   *
   * @param msg message to be sent
   * @param timeout_ms timeout in milliseconds
   * @return std::shared_ptr<DTXMessage> the response message
   */
  virtual std::shared_ptr<DTXMessage> SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                                      uint32_t timeout_ms = -1) = 0;

  /**
   * Send message asynchronously
   *
   * @param msg message to be sent
   * @param callback callback for response message
   */
  virtual void SendMessageAsync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) = 0;

  /**
   * Cancel the channel
   * TODO: move it method out of this interface
   *
   * @param channel
   * @return succeed or fail
   */
  virtual bool CancelChannel(const DTXChannel& channel) = 0;
};

}  // namespace idevice

#endif  // IDEVICE_INSTRUMENT_DTXMESSENGER_H
