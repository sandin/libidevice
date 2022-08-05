#ifndef IDEVICE_DTXSERVICE_H
#define IDEVICE_DTXSERVICE_H

#include <cstdint>
#include <cstdio>

#include "idevice/dtxtransport.h"
#include "idevice/dtxmessage.h"
#include "idevice/dtxmessageparser.h"
#include "idevice/dtxmessagetransmitter.h"

namespace idevice {

class DTXService {
 public:
  DTXService() {}
  virtual ~DTXService() {}

  // bool SendMessageSync(const DTXMessage& msg, ReplyHandler callback);
  // bool SendMessageAsync(const DTXMessage& msg, ReplyHandler callback);

 private:
  // send_queue_;
  // receive_queue_;
  DTXTransport transport_;
  DTXMessageParser incoming_parser_;
  DTXMessageTransmitter outgoing_transmitter_;

}; // class DTXService

}  // namespace idevice

#endif // IDEVICE_DTXSERVICE_H
