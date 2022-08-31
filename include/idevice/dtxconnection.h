#ifndef IDEVICE_DTXCONNECTION_H
#define IDEVICE_DTXCONNECTION_H

#include <atomic>
#include <memory>  // std::unique_ptr, std::shared_ptr
#include <thread>
#include <unordered_map>
#include <utility>  // std::pair

#include "idevice/blockingqueue.h"
#include "idevice/dtxchannel.h"
#include "idevice/dtxmessage.h"
#include "idevice/dtxmessageparser.h"
#include "idevice/dtxmessagetransmitter.h"
#include "idevice/dtxmessenger.h"
#include "idevice/dtxtransport.h"

namespace idevice {

class DTXConnection : public DTXMessenger {
 public:
  using ChannelIdentifier = uint32_t;
  using MessageIdentifier = uint32_t;
  using ReplyIdentifier = uint64_t;  // ChannelIdentifier << 32 || MessageIdentifier
  using DTXMessageWithRoutingInfo = std::pair<std::shared_ptr<DTXMessage>, DTXMessageRoutingInfo>;

  DTXConnection(IDTXTransport* transport) : transport_(transport) {}
  virtual ~DTXConnection() {
  }

  bool Connect();
  bool Disconnect();
  bool IsConnected() const { return transport_->IsConnected(); }

  std::shared_ptr<DTXChannel> MakeChannelWithIdentifier(const std::string& channel_identifier);
  bool CannelChannel(std::shared_ptr<DTXChannel> channel);

  virtual std::shared_ptr<DTXMessage> SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                                      const DTXChannel& channel,
                                                      uint32_t timeout_ms = -1) override;
  virtual void SendMessageAsync(std::shared_ptr<DTXMessage> msg, const DTXChannel& channel,
                                ReplyHandler callback) override;

  void DumpStat() const;  // ONLY FOR DEBUG

 private:
  struct Packet {
    char* buffer;
    size_t size;
  };

  void StartSendThread();
  void SendThread();
  void StopSendThread(bool await);

  void StartReceiveThread();
  void ReceiveThread();
  void StopReceiveThread(bool await);

  void StartParsingThread();
  void ParsingThread();
  void StopParsingThread(bool await);

  void RouteMessage(std::shared_ptr<DTXMessage> msg);

  std::atomic_bool send_thread_running_ = ATOMIC_VAR_INIT(false);
  std::unique_ptr<std::thread> send_thread_ = nullptr;  /// sender of outgoing messages

  std::atomic_bool receive_thread_running_ = ATOMIC_VAR_INIT(false);
  std::unique_ptr<std::thread> receive_thread_ = nullptr;  ///< producer of incoming packets

  std::atomic_bool parsing_thread_running_ = ATOMIC_VAR_INIT(false);
  std::unique_ptr<std::thread> parsing_thread_ = nullptr;  ///< consumer of incoming packets

  BlockingQueue<DTXMessageWithRoutingInfo> send_queue_;
  BlockingQueue<std::unique_ptr<Packet>> receive_queue_;

  std::atomic<ChannelIdentifier> next_channel_code_ = ATOMIC_VAR_INIT(1);
  std::unordered_map<ChannelIdentifier, std::shared_ptr<DTXChannel>> channels_by_code_;

  std::unordered_map<ReplyIdentifier, ReplyHandler> _handlers_by_identifier_;

  std::atomic<MessageIdentifier> next_msg_identifier_ = ATOMIC_VAR_INIT(1);

  IDTXTransport* transport_;
  DTXMessageParser incoming_parser_;
  DTXMessageTransmitter outgoing_transmitter_;

  DTXChannel default_channel_;

};  // class DTXConnection

}  // namespace idevice

#endif  // IDEVICE_DTXCONNECTION_H
