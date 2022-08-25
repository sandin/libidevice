#ifndef IDEVICE_BLOCKING_QUEUE_H
#define IDEVICE_BLOCKING_QUEUE_H

#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional> // std::function

#include "idevice/macro_def.h" // IDEVICE_DISALLOW_COPY_AND_ASSIGN

namespace idevice {

/**
 * A blocking queue
 */
template <typename T>
class BlockingQueue {
 public:
  BlockingQueue() {}
  ~BlockingQueue() {}
  
  IDEVICE_DISALLOW_COPY_AND_ASSIGN(BlockingQueue);
  
  /**
   * add(copy) a new element to the end of the queue
   * @param data new element
   */
  void Push(const T& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(data); // copy
    not_empty_.notify_all();
  }
  
  /**
   * add(move) a new element to the end of the queue
   * @param data new element
   */
  void Push(T&& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(std::forward<T>(data)); // move
    not_empty_.notify_all();
  }
  
  /**
   * take(move) the first element from the head of the queue
   * if the queue is empty, it will wait blocking.
   * @return the first element
   */
  T Take() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      not_empty_.wait(lock);
    }
    T data = std::move(queue_.front()); // move
    queue_.pop();
    return data; // move
  }
  
  /**
   * waiting queue is not empty with a timeout
   * @param timeout_ms timeout
   * @param return true if the queue is not empty, and ready to take
   */
  bool WaitToTake(uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      if (not_empty_.wait_for(lock, std::chrono::microseconds(timeout_ms)) == std::cv_status::timeout) {
        return false;
      }
    }
    return true;
  }
  
  /**
   * checks whether the queue is empty
   * @return return true if the queue is empty
   */
  bool Empty() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
  }
  
  /**
   * clear the queue
   */
  void Clear(std::function<void(T&)> destructor = nullptr) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (destructor) {
      while (!queue_.empty()) {
        destructor(queue_.front());
        queue_.pop();
      }
    } else {
      std::queue<T> empty;
      std::swap(queue_, empty);
    }
  }
  
private:
  mutable std::mutex mutex_;
  std::queue<T> queue_;
  std::condition_variable not_empty_;
  
}; // class BlockingQueue

}  // namespace idevice

#include "idevice/macro_undef.h"

#endif // IDEVICE_BLOCKING_QUEUE_H
