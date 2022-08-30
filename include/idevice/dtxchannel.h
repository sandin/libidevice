#ifndef IDEVICE_DTXCHANNEL_H
#define IDEVICE_DTXCHANNEL_H

#include <cstdint>  // uint32_t
#include <string>

#include "idevice/dtxmessenger.h"

namespace idevice {

class DTXChannel /*: public DTXMessenger */ {
 public:
  DTXChannel(DTXMessenger* connection, const std::string& label, uint32_t channel_identifier)
      : connection_(connection), label_(label), channel_identifier_(channel_identifier) {}
  DTXChannel() : DTXChannel(nullptr, "", 0) {}
  virtual ~DTXChannel() {}

  // virtual bool SendMessageSync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) override;
  void SendMessageAsync(std::shared_ptr<DTXMessage> msg, DTXMessenger::ReplyHandler callback);

  const std::string& Label() const { return label_; }
  uint32_t ChannelIdentifier() const { return channel_identifier_; }

 private:
  std::string label_ = "";
  uint32_t channel_identifier_ = 0;
  DTXMessenger* connection_ = nullptr;

};  // class DTXChannel

}  // namespace idevice

#endif  // IDEVICE_DTXCHANNEL_H
