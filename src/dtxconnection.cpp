#include "idevice/dtxconnection.h"

#include <algorithm> // std::max
#include <future> // std::promise

#include "idevice/macro_def.h" // IDEVICE_START_THREAD, IDEVICE_STOP_THREAD, IDEVICE_ATOMIC_SET_MAX, IDEVICE_DTXMESSAGE_IDENTIFIER

using namespace idevice;

static const size_t kReceiveBufferSize = 16 * 1024; // 0x4000(16384)
static const uint32_t kReceiveTimeout = 1 * 1000;

bool DTXConnection::Connect() {
  bool ret = transport_->Connect();
  if (ret) {
    StartSendThread();
    StartParsingThread();
    StartReceiveThread();
  }
  return ret;
}

bool DTXConnection::Disconnect() {
  StopSendThread(false);
  StopReceiveThread(false);
  StopParsingThread(false);
  
  send_queue_.Clear();
  receive_queue_.Clear([](std::unique_ptr<Packet>& packet) { free(packet->buffer); });
  
  return transport_->Disconnect();
}

std::shared_ptr<DTXChannel> DTXConnection::MakeChannelWithIdentifier(const std::string& channel_identifier) {
  uint32_t channel_code = next_channel_code_.fetch_add(1);
  std::shared_ptr<DTXChannel> channel = std::make_shared<DTXChannel>(this, channel_identifier, channel_code);
  channels_by_code_.insert(std::make_pair(channel_code, channel));
  
  std::shared_ptr<DTXMessage> message = DTXMessage::Create("_requestChannelWithCode:identifier:");
  message->AppendAuxiliary(DTXPrimitiveValue(static_cast<int32_t>(channel_code)));
  message->AppendAuxiliary(nskeyedarchiver::KAValue(channel_identifier.c_str()));
  // TODO: set Message Routing Info
  
  std::shared_ptr<DTXMessage> response = SendMessageSync(message, default_channel_, -1 /* wait forever */);
#if IDEVICE_DEBUG
  IDEVICE_LOG_D("response message:\n");
  response->Dump();
#endif
  return channel;
}

bool DTXConnection::CannelChannel(std::shared_ptr<DTXChannel> channel) {
  // TODO:
  // channels_by_code_.remove channel
}

void DTXConnection::DumpStat() const {
  printf("==== DTXConnection Stat ====\n");
  printf("send_thread_ running: %d\n", send_thread_running_.load());
  printf("receive_thread_ running: %d\n", receive_thread_running_.load());
  printf("parsing_thread_ running: %d\n", parsing_thread_running_.load());
  printf("send_queue_.size: %zu\n", send_queue_.Size());
  printf("receive_queue_.size: %zu\n", receive_queue_.Size());
  printf("next_channel_code_: %d\n", next_channel_code_.load());
  printf("next_msg_identifier_: %d\n", next_msg_identifier_.load());
  printf("channels_by_code_:\n");
  for (const auto& item : channels_by_code_) {
    printf("\tchannel code: %d, label: %s\n", item.first, item.second->Label().c_str());
  }
  printf("_handlers_by_identifier_:\n");
  for (const auto& item : _handlers_by_identifier_) {
    printf("\tcallback identifier: %llx, callback function ptr: %p\n", item.first, &item.second);
  }
  printf("==== /DTXConnection Stat ====\n");
}

#pragma mark - Send Messages

/*
 * The life of a message:
 *
 * ┌────────────┐     ┌────────────┐
 * │    Msg     │     │  Callback  │
 * └─────┬──────┘     └────────────┘
 *       │                   ▲
 *       ▼                   │
 * ┌────────────┐     ┌──────┴─────┐
 * │  SendQueue │     │ ParseQueue │
 * └─────┬──────┘     └────────────┘
 *       │                   ▲
 *       ▼                   │
 * ┌────────────┐     ┌──────┴─────┐
 * │  Transport │     │ReceiveQueue│
 * └─────┬──────┘     └────────────┘
 *       │                   ▲
 *       ▼                   │
 * ┌────────────┐     ┌──────┴─────┐
 * │ DTXService ├────►│  RespMsg   │
 * └────────────┘     └────────────┘
 */
void DTXConnection::SendMessageAsync(std::shared_ptr<DTXMessage> msg, const DTXChannel& channel, ReplyHandler callback) {
  // put the message into the send queue
  DTXMessageRoutingInfo routing_info = { 0 };
  routing_info.msg_identifier = next_msg_identifier_.fetch_add(1);
  routing_info.channel_code = channel.ChannelIdentifier();
  routing_info.conversation_index = 0; // TODO
  routing_info.expects_reply = callback != nullptr;
  
  send_queue_.Push(std::make_pair(msg, std::move(routing_info)));
  
  // and save the callback of the message
  if (callback != nullptr) {
    uint64_t reply_identifier = IDEVICE_DTXMESSAGE_IDENTIFIER(routing_info.channel_code, routing_info.msg_identifier);
    _handlers_by_identifier_.insert(std::make_pair(reply_identifier, callback));
  }
}

std::shared_ptr<DTXMessage> DTXConnection::SendMessageSync(std::shared_ptr<DTXMessage> msg, const DTXChannel& channel, uint32_t timeout_ms) {
  std::promise<std::shared_ptr<DTXMessage>> promise;
  std::future<std::shared_ptr<DTXMessage>> future = promise.get_future();
  SendMessageAsync(msg, channel, [&promise](auto response_msg) {
    promise.set_value(response_msg);
  });
  if (timeout_ms != -1) {
    return future.get();
  } else {
    if (future.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout) {
      return nullptr;
    }
    return future.get();
  }
}

void DTXConnection::StartSendThread() {
  IDEVICE_START_THREAD(send_thread_, &DTXConnection::SendThread, send_thread_running_);
}

void DTXConnection::StopSendThread(bool await) {
  IDEVICE_STOP_THREAD(send_thread_, send_thread_running_, await);
}

void DTXConnection::SendThread() {
  IDEVICE_LOG_I("SendThread start\n");
  constexpr size_t send_buffer_size = 8 * 1024;
  char* send_buffer = static_cast<char*>(malloc(send_buffer_size));
  while (send_thread_running_.load(std::memory_order_acquire)) {
    if (!IsConnected()) {
      return;
    }
    
    DTXMessageWithRoutingInfo message_with_routing_info = send_queue_.Take();
    std::shared_ptr<DTXMessage> message = message_with_routing_info.first;
    const DTXMessageRoutingInfo& routing_info = message_with_routing_info.second;
    
    BufferedDTXTransport buffered_transport(transport_, send_buffer, send_buffer_size);
    bool ret = outgoing_transmitter_.TransmitMessage(message, 0, routing_info, [&](const char* buffer, size_t size) -> bool {
      return buffered_transport.Send(buffer, size);
    });
    buffered_transport.Flush();
  
    if (!ret) { // TODO: can we trust this return value?
      IDEVICE_LOG_E("Error: can not send outgoing message, diconnecting.\n");
      Disconnect();
      break;
    }
    
    // TODO: callback with routing info
    
  }
  free(send_buffer);
  IDEVICE_LOG_I("SendThread stop\n");
}



#pragma mark - Receive Messages

void DTXConnection::StartReceiveThread() {
  IDEVICE_START_THREAD(receive_thread_, &DTXConnection::ReceiveThread, receive_thread_running_);
}

void DTXConnection::StopReceiveThread(bool await) {
  IDEVICE_STOP_THREAD(receive_thread_, receive_thread_running_, await);
}

void DTXConnection::ReceiveThread() {
  IDEVICE_LOG_I("ReceiveThread start\n");
  std::unique_ptr<Packet> receive_packet = nullptr;
  while (receive_thread_running_.load(std::memory_order_acquire)) {
    if (!IsConnected()) {
      return;
    }
    
    if (!receive_packet) {
      receive_packet = std::make_unique<Packet>();
      receive_packet->buffer = static_cast<char*>(malloc(kReceiveBufferSize)); // the customer is responsible for freeing it
      receive_packet->size = 0;
    }
    
    if (!transport_->ReceiveWithTimeout(receive_packet->buffer, kReceiveBufferSize, kReceiveTimeout, (uint32_t*)&receive_packet->size)) {
      IDEVICE_LOG_E("Error: Receive ret != 0\n");
      break;
    }
    
    if (receive_packet->size > 0) {
      IDEVICE_LOG_V("received %zu bytes\n", receive_packet->size);
      receive_queue_.Push(std::move(receive_packet));
    }
    
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    std::this_thread::yield();
  }
  
  if (receive_packet) {
    free(receive_packet->buffer);
  }
  
  //if (IsConnected()) {
  //  Disconnect(); // TODO:
  //}
  IDEVICE_LOG_I("ReceiveThread stop\n");
}

#pragma mark - Parsing Messages

void DTXConnection::StartParsingThread() {
  IDEVICE_START_THREAD(parsing_thread_, &DTXConnection::ParsingThread, parsing_thread_running_);
}

void DTXConnection::StopParsingThread(bool await) {
  IDEVICE_STOP_THREAD(parsing_thread_, parsing_thread_running_, await);
}

void DTXConnection::ParsingThread() {
  IDEVICE_LOG_I("ParsingThread start\n");
  while (parsing_thread_running_.load(std::memory_order_acquire)) {
    if (!IsConnected()) {
      return;
    }
    
    std::unique_ptr<Packet> packet = receive_queue_.Take();
    IDEVICE_LOG_D("parsing %zu bytes\n", packet->size);
    bool ret = incoming_parser_.ParseIncomingBytes(packet->buffer, packet->size);
    free(packet->buffer); // all data in the packet buffer has been copied to the parser buffer
    
    if (!ret) {
      IDEVICE_LOG_E("Error: can not parse incoming bytes, diconnecting.\n");
      Disconnect();
      break;
    }
    
    std::vector<std::shared_ptr<DTXMessage>> messages = incoming_parser_.PopAllParsedMessages();
    uint32_t max_msg_identifier = 0;
    for (auto& msg : messages) {
      RouteMessage(msg);
      max_msg_identifier = std::max(max_msg_identifier, msg->Identifier());
    }
    IDEVICE_ATOMIC_SET_MAX(next_msg_identifier_, max_msg_identifier + 1);
  }
  IDEVICE_LOG_I("ParsingThread stop\n");
}

void DTXConnection::RouteMessage(std::shared_ptr<DTXMessage> msg) {
  uint32_t msg_identifier = msg->Identifier();
  uint32_t channel_code = msg->ChannelCode();
  uint64_t callback_identifier = IDEVICE_DTXMESSAGE_IDENTIFIER(channel_code, msg_identifier);
  
  auto callback_found = _handlers_by_identifier_.find(callback_identifier);
  if (callback_found != _handlers_by_identifier_.end()) {
    auto callback = callback_found->second;
    callback(msg); // -> invoke callback with the parsed message TODO: the callback may slow down the parser, and thus make the receive queue larger.
    // TODO: notify all
    // TODO: release anything associated with this message
    _handlers_by_identifier_.erase(callback_found);
    return;
  }
  
  auto found = channels_by_code_.find(channel_code);
  if (found != channels_by_code_.end()) {
    std::shared_ptr<DTXChannel> channel = found->second;
    /* TODO
    auto callback = channel.GetMessageHandler();
    if (callback != nullptr) {
      callback(msg);
    }
    */
  } else {
    IDEVICE_LOG_I("dropped message (no message handler). channel code: %d, msg identifier: %d\n", channel_code, msg_identifier);
  }
}




