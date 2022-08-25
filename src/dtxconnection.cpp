#include "idevice/dtxconnection.h"

#include <algorithm> // std::max

#include "idevice/macro_def.h"

using namespace idevice;

static const size_t kReceiveBufferSize = 16 * 1024;
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

void DTXConnection::StartReceiveThread() {
  IDEVICE_START_THREAD(receive_thread_, &DTXConnection::ReceiveThread, receive_thread_running_);
}

void DTXConnection::StopReceiveThread(bool await) {
  IDEVICE_STOP_THREAD(receive_thread_, receive_thread_running_, await);
}

void DTXConnection::ReceiveThread() {
  printf("ReceiveThread start\n");
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
      printf("Error: Receive ret != 0\n");
      break;
    }
    
    if (receive_packet->size > 0) {
      printf("received %zu bytes\n", receive_packet->size);
      receive_queue_.Push(std::move(receive_packet));
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1)); // TODO: delete me
  }
  
  if (receive_packet) {
    free(receive_packet->buffer);
  }
  
  //if (IsConnected()) {
  //  Disconnect(); // TODO:
  //}
  printf("ReceiveThread stop\n");
}

void DTXConnection::StartParsingThread() {
  IDEVICE_START_THREAD(parsing_thread_, &DTXConnection::ParsingThread, parsing_thread_running_);
}

void DTXConnection::StopParsingThread(bool await) {
  IDEVICE_STOP_THREAD(parsing_thread_, parsing_thread_running_, await);
}

void DTXConnection::ParsingThread() {
  printf("ParsingThread start\n");
  while (parsing_thread_running_.load(std::memory_order_acquire)) {
    if (!IsConnected()) {
      return;
    }
    
    std::unique_ptr<Packet> packet = receive_queue_.Take();
    printf("parsing %zu bytes\n", packet->size);
    bool ret = incoming_parser_.ParseIncomingBytes(packet->buffer, packet->size);
    free(packet->buffer); // all data in the packet buffer has been copied to the parser buffer
    
    if (!ret) {
      printf("Error: can not parse incoming bytes, diconnecting.\n");
      Disconnect();
      break;
    }
    
    std::vector<std::shared_ptr<DTXMessage>> messages = incoming_parser_.PopAllParsedMessages();
    uint32_t max_identifier = 0;
    for (auto& msg : messages) {
      max_identifier = std::max(max_identifier, msg->Identifier());
      // TODO: callback with routing info
    }
    
    IDEVICE_ATOMIC_SET_MAX(next_msg_identifier_, max_identifier);
  }
  printf("ParsingThread stop\n");
}

void DTXConnection::StartSendThread() {
  IDEVICE_START_THREAD(send_thread_, &DTXConnection::SendThread, send_thread_running_);
}

void DTXConnection::StopSendThread(bool await) {
  IDEVICE_STOP_THREAD(send_thread_, send_thread_running_, await);
}

void DTXConnection::SendThread() {
  printf("SendThread start\n");
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
    bool ret = outgoing_transmitter_.TransmitMessage(message, 0, {0, 0} /* TODO */, [&](const char* buffer, size_t size) -> bool {
      return buffered_transport.Send(buffer, size);
    });
    buffered_transport.Flush();
  
    if (!ret) { // TODO: can we trust this return value?
      printf("Error: can not send outgoing message, diconnecting.\n");
      Disconnect();
      break;
    }
    
    // TODO: callback with routing info
    
  }
  free(send_buffer);
  printf("SendThread stop\n");
}

std::shared_ptr<DTXChannel> DTXConnection::MakeChannelWithIdentifier(const std::string& channel_identifier) {
  uint32_t channel_code = next_channel_code_.fetch_add(1);
  std::shared_ptr<DTXChannel> channel = std::make_shared<DTXChannel>(this, channel_identifier, channel_code);
  channels_by_code_.insert(std::make_pair(channel_code, channel));
  
  std::shared_ptr<DTXMessage> message = DTXMessage::Create("_requestChannelWithCode:identifier");
  message->AppendAuxiliary(DTXPrimitiveValue(static_cast<int32_t>(channel_code)));
  message->AppendAuxiliary(nskeyedarchiver::KAValue(channel_identifier.c_str()));
  // TODO: set Message Routing Info
  
  SendMessageAsync(message, default_channel_, [&](auto msg) {
    printf("message reply handler\n");
    // TODO:
  });
  
  return channel;
}

bool DTXConnection::CannelChannel(std::shared_ptr<DTXChannel> channel) {
  // TODO:
  // channels_by_code_.remove channel
}

// virtual override
void DTXConnection::SendMessageAsync(std::shared_ptr<DTXMessage> msg, const DTXChannel& channel, ReplyHandler callback) {
  uint32_t identifier = next_msg_identifier_.fetch_add(1);
  
  DTXMessageRoutingInfo routing_info;
  msg->SetChannelCode(channel.ChannelIdentifier());
  msg->SetIdentifier(identifier);
  // TODO: msg->SetConversationIndex();
  send_queue_.Push(std::make_pair(msg, std::move(routing_info)));
  // TODO: store callback of message
}

