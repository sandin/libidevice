#ifndef IDEVICE_DTXMESSENGER_H
#define IDEVICE_DTXMESSENGER_H

#include <functional>

namespace idevice {

class DTXMessage;

// Interface
class DTXMessenger {
 public:
  using ReplyHandler = std::function<void(std::shared_ptr<DTXMessage>)>;
  
  virtual ~DTXMessenger() {}
  
  // virtual bool SendMessageSync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) = 0;
  virtual void SendMessageAsync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) = 0;
}; 

}  // namespace idevice

#endif // IDEVICE_DTXMESSENGER_H
