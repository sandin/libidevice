#ifndef IDEVICE_INSTRUMENT_DTXCONNECTION_H
#define IDEVICE_INSTRUMENT_DTXCONNECTION_H

#include <atomic>
#include <memory>  // std::unique_ptr, std::shared_ptr
#include <thread>
#include <unordered_map>
#include <utility>  // std::pair

#include "idevice/utils/blockingqueue.h"
#include "idevice/instrument/dtxchannel.h"
#include "idevice/instrument/dtxmessage.h"
#include "idevice/instrument/dtxmessageparser.h"
#include "idevice/instrument/dtxmessagetransmitter.h"
#include "idevice/instrument/dtxmessenger.h"
#include "idevice/instrument/dtxtransport.h"

namespace idevice {

/**
 * The connection for communication with instrument service using DTXMessage protocol.
 */
class DTXConnection : public DTXMessenger {
 public:
  using ChannelIdentifier = uint32_t;
  using MessageIdentifier = uint32_t;
  using ReplyIdentifier = uint64_t;  // ChannelIdentifier << 32 || MessageIdentifier
  using DTXMessageWithRoutingInfo = std::pair<std::shared_ptr<DTXMessage>, DTXMessageRoutingInfo>;
 
  /**
   * Constructor
   * 
   * @param transport A transport
   */
  DTXConnection(IDTXTransport* transport) : transport_(transport) {}

  /**
   * Destructor
   */
  virtual ~DTXConnection() {
    // IDEVICE_ASSERT(!IsConnected());
  }

  /**
   * Connect to the service 
   * 
   * @return succeed or fail 
   */
  bool Connect();

  /**
   * Disconnect from the service
   * 
   * @return succeed or fail  
   */
  bool Disconnect();

  /**
   * Check whether it's connected or not
   * 
   * @return connected or not 
   */
  bool IsConnected() const { return transport_->IsConnected(); }

  /**
   * Request service to open a new channel
   * 
   * @param channel_identifier channel code to identify the channel
   * @return std::shared_ptr<DTXChannel> the channel that was just created
   */
  std::shared_ptr<DTXChannel> MakeChannelWithIdentifier(const std::string& channel_identifier);
  
  /**
   * Close a channel 
   * 
   * @param channel the channel to be closed
   * @return succeed or fail 
   */
  virtual bool CancelChannel(const DTXChannel& channel) override;

  /**
   * Send message synchronously
   *
   * @param msg message to be sent
   * @param timeout_ms timeout in milliseconds
   * @return std::shared_ptr<DTXMessage> the response message
   */
  virtual std::shared_ptr<DTXMessage> SendMessageSync(std::shared_ptr<DTXMessage> msg,
                                                      uint32_t timeout_ms = -1) override;

  /**
   * Send message asynchronously
   *
   * @param msg message to be sent
   * @param callback callback for response message
   */
  virtual void SendMessageAsync(std::shared_ptr<DTXMessage> msg,
                                ReplyHandler callback) override;

  /**
   * Dump all stat of this connection 
   * Used for debugging
   */
  void DumpStat() const; 

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
  void ReplyMessage(std::shared_ptr<DTXMessage> msg);

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

#endif  // IDEVICE_INSTRUMENT_DTXCONNECTION_H
