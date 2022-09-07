#include "idevice/instrument/dtxchannel.h"

#include "idevice/instrument/dtxmessage.h"

using namespace idevice;

std::shared_ptr<DTXMessage> DTXChannel::SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                                        uint32_t timeout_ms) {
  msg->SetChannelCode(channel_identifier_);
  return connection_->SendMessageSync(msg, timeout_ms);
}

void DTXChannel::SendMessageAsync(std::shared_ptr<DTXMessage> msg,
                                  DTXMessenger::ReplyHandler callback) {
  msg->SetChannelCode(channel_identifier_);
  connection_->SendMessageAsync(msg, callback);
}

void DTXChannel::Cancel() {
  canceled_ = connection_->CancelChannel(*this);
}
