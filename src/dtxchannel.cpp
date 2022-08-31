#include "idevice/dtxchannel.h"

using namespace idevice;

std::shared_ptr<DTXMessage> DTXChannel::SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                                        uint32_t timeout_ms) {
  return connection_->SendMessageSync(msg, *this, timeout_ms);
}

void DTXChannel::SendMessageAsync(std::shared_ptr<DTXMessage> msg,
                                  DTXMessenger::ReplyHandler callback) {
  connection_->SendMessageAsync(msg, *this, callback);
}

void DTXChannel::Cancel() {
  canceled_ = connection_->CancelChannel(*this);
}
