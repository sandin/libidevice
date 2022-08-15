#ifndef IDEVICE_DTXCONNECTION_H
#define IDEVICE_DTXCONNECTION_H

#include <thread>
#include <memory>
#include <atomic>

#include "idevice/blockingqueue.h"
#include "idevice/dtxtransport.h"
#include "idevice/dtxmessenger.h"
#include "idevice/dtxmessage.h"
#include "idevice/dtxchannel.h"
#include "idevice/dtxmessageparser.h"
#include "idevice/dtxmessagetransmitter.h"

namespace idevice {



class DTXConnection : public DTXMessenger {
 public:
  DTXConnection(DTXTransport* transport) : transport_(transport) {}
  virtual ~DTXConnection() {}
  
  bool Connect();
  bool Disconnect();
  bool IsConnected() const { return transport_->IsConnected(); }
  
  DTXChannel* MakeChannelWithIdentifier(const std::string& channel_identifier);
  bool CannelChannel(DTXChannel* channel);

  // virtual bool SendMessageSync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) override;
  // virtual bool SendMessageAsync(std::shared_ptr<DTXMessage> msg, ReplyHandler callback) override;

 private:
  struct Packet {
    char* buffer;
    size_t size;
  };
  
  void StartReceiveThread();
  void ReceiveThread();
  void StopReceiveThread(bool await);
  
  void StartParsingThread();
  void ParsingThread();
  void StopParsingThread(bool await);
  
  std::atomic_bool receive_thread_running_ = ATOMIC_VAR_INIT(false);
  std::unique_ptr<std::thread> receive_thread_ = nullptr; ///< producer of packets
  
  std::atomic_bool parsing_thread_running_ = ATOMIC_VAR_INIT(false);
  std::unique_ptr<std::thread> parsing_thread_ = nullptr; ///< consumer of packets
  
  // TODO: send_queue_;
  BlockingQueue<std::unique_ptr<Packet>> receive_queue_;
  
  DTXTransport* transport_;
  DTXMessageParser incoming_parser_;
  DTXMessageTransmitter outgoing_transmitter_;

}; // class DTXConnection

}  // namespace idevice

#endif // IDEVICE_DTXCONNECTION_H
