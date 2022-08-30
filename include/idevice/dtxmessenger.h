#ifndef IDEVICE_DTXMESSENGER_H
#define IDEVICE_DTXMESSENGER_H

#include <functional>

namespace idevice {

class DTXMessage;
class DTXChannel;

// Interface
class DTXMessenger {
 public:
  using ReplyHandler = std::function<void(std::shared_ptr<DTXMessage>)>;

  virtual ~DTXMessenger() {}

  virtual std::shared_ptr<DTXMessage> SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                                      const DTXChannel& channel,
                                                      uint32_t timeout_ms = -1) = 0;
  virtual void SendMessageAsync(std::shared_ptr<DTXMessage> msg, const DTXChannel& channel,
                                ReplyHandler callback) = 0;
};

}  // namespace idevice

#endif  // IDEVICE_DTXMESSENGER_H
