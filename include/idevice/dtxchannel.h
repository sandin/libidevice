#ifndef IDEVICE_DTXCHANNEL_H
#define IDEVICE_DTXCHANNEL_H

#include <cstdint>
#include <cstdio>

#include "idevice/dtxmessenger.h"

namespace idevice {

class DTXChannel : public DTXMessenger {
 public:
  DTXChannel(DTXMessenger* connection, const std::string& label, uint32_t channel_code) : connection_(connection), label_(label), channel_code_(channel_code) {}
  virtual ~DTXChannel() {}

  // virtual bool SendMessageSync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) override;
  // virtual bool SendMessageAsync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) override;
  
  const std::string& Label() const { return label_; }
  uint32_t ChannelCode() const { return channel_code_; }

 private:
  std::string label_;
  uint32_t channel_code_;
  DTXMessenger* connection_;

}; // class DTXChannel

}  // namespace idevice

#endif // IDEVICE_DTXCHANNEL_H
