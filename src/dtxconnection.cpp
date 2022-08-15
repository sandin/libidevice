#include "idevice/dtxconnection.h"

using namespace idevice;

static const size_t kReceiveBufferSize = 16 * 1024;
static const uint32_t kReceiveTimeout = 1 * 1000;

#define DTXCONNECTION_START_THREAD(thread_var, thread_func, stop_flag) \
  stop_flag.store(true, std::memory_order_release); \
  thread_var = std::make_unique<std::thread>(std::bind(&DTXConnection::thread_func, this));

#define DTXCONNECTION_STOP_THREAD(thread_var, stop_flag, await) \
  if (stop_flag.load(std::memory_order_acquire)) { \
    stop_flag.store(false, std::memory_order_release); \
    if (await) { \
      thread_var->join(); \
    } \
  } \

bool DTXConnection::Connect() {
  bool ret = transport_->Connect();
  if (ret) {
    StartParsingThread();
    StartReceiveThread();
  }
  return ret;
}

bool DTXConnection::Disconnect() {
  StopReceiveThread(false);
  StopParsingThread(false);
  receive_queue_.Clear([](std::unique_ptr<Packet>& packet) { free(packet->buffer); });
  
  return transport_->Disconnect();
}

void DTXConnection::StartReceiveThread() {
  DTXCONNECTION_START_THREAD(receive_thread_, ReceiveThread, receive_thread_running_);
}

void DTXConnection::StopReceiveThread(bool await) {
  DTXCONNECTION_STOP_THREAD(receive_thread_, receive_thread_running_, await);
}

void DTXConnection::ReceiveThread() {
  printf("ReceiveThread start\n");
  while (receive_thread_running_.load(std::memory_order_acquire)) {
    if (!IsConnected()) {
      return;
    }
    
    std::unique_ptr<Packet> receive_packet = std::make_unique<Packet>();
    receive_packet->buffer = static_cast<char*>(malloc(kReceiveBufferSize)); // the customer is responsible for freeing it
    receive_packet->size = 0;
    
    if (!transport_->ReceiveWithTimeout(receive_packet->buffer, kReceiveBufferSize, kReceiveTimeout, (uint32_t*)&receive_packet->size)) {
      printf("Error: Receive ret != 0\n");
      free(receive_packet->buffer);
      break;
    }
    
    printf("received %zu bytes\n", receive_packet->size);
    receive_queue_.Push(std::move(receive_packet));
  }
  
  //if (IsConnected()) {
  //  Disconnect(); // TODO:
  //}
  printf("ReceiveThread stop\n");
}

void DTXConnection::StartParsingThread() {
  DTXCONNECTION_START_THREAD(parsing_thread_, ParsingThread, parsing_thread_running_);
}

void DTXConnection::StopParsingThread(bool await) {
  DTXCONNECTION_STOP_THREAD(parsing_thread_, parsing_thread_running_, await);
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
    free(packet->buffer);
    
    if (!ret) {
      printf("Error: can not parse incoming bytes, diconnecting.\n");
      Disconnect();
      break;
    }
  }
  printf("ParsingThread stop\n");
}
