#include "idevice/dtxchannel.h"

using namespace idevice;

// virtual override
void DTXChannel::SendMessageAsync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) {
  connection_->SendMessageAsync(msg, callback);
}
