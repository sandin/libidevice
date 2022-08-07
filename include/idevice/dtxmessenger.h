#ifndef IDEVICE_DTXMESSENGER_H
#define IDEVICE_DTXMESSENGER_H

namespace idevice {

// Interface
class DTXMessenger {
 public:
  virtual ~DTXMessenger() {}
  // virtual bool SendMessageSync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) = 0;
  // virtual bool SendMessageAsync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) = 0;
}; 

}  // namespace idevice

#endif // IDEVICE_DTXMESSENGER_H
