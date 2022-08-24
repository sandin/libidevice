#include "idevice/dtxchannel.h"

using namespace idevice;

void DTXChannel::SendMessageAsync(std::shared_ptr<DTXMessage> msg, DTXMessenger::ReplyHandler callback) {
  connection_->SendMessageAsync(msg, *this, callback);
}
