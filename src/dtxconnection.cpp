#include "idevice/dtxconnection.h"

using namespace idevice;

static const size_t kReceiveBufferSize = 16 * 1024;
static const uint32_t kReceiveTimeout = 1000;

bool DTXConnection::Connect() {
  bool ret = transport_->Connect();
  if (ret) {
    StartReceiveThread();
  }
  return ret;
}

bool DTXConnection::Disconnect() {
  return transport_->Disconnect();
}

void DTXConnection::StartReceiveThread() {
  receive_thread_running_.store(true, std::memory_order_release);
  receive_thread_ = std::make_unique<std::thread>(std::bind(&DTXConnection::ReceiveThread, this));
}

void DTXConnection::StopReceiveThread(bool await) {
  if (receive_thread_running_.load(std::memory_order_acquire)) {
    receive_thread_running_.store(false, std::memory_order_release);
    if (await) {
      receive_thread_->join();
    }
    receive_thread_.reset();
  }
}

void DTXConnection::ReceiveThread() {
  printf("ReceiveThread\n");
  char* receive_buffer = static_cast<char*>(malloc(kReceiveBufferSize));
  uint32_t received = 0;
  while (receive_thread_running_.load(std::memory_order_acquire)) {
    printf("ReceiveWithTimeout\n");
    if (!transport_->ReceiveWithTimeout(receive_buffer, kReceiveBufferSize, kReceiveTimeout, &received)) {
      printf("Error: Receive ret != 0\n");
      break;
    }
    
    printf("received: %d\n", received);
    incoming_parser_.ParseIncomingBytes(receive_buffer, received);
  }
  free(receive_buffer);
}

