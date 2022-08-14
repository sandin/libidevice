#include "idevice/blockingqueue.h"

#include <map>
#include <gtest/gtest.h>

#include <thread>

using namespace idevice;

// Item of BlockingQueue for testing
class Task {
public:
  Task(int id) : id_(id) {
    printf("Task(int id), id=%d\n", id_);
  }
  ~Task() {
    printf("~Task(), id=%d\n", id_);
  }
  
  // copy ctor and assign
  Task(const Task& other) : id_(other.id_) {
    printf("Task(const Task& other), other.id=%d\n", other.id_);
  }
  void operator ==(const Task& other) {
    printf("operator ==(const Task& other), other.id=%d\n", other.id_);
    id_ = other.id_;
  }
  
  // move ctor and assign
  Task(Task&& other) : id_(other.id_) {
    printf("Task(Task&& other), other.id=%d\n", other.id_);
    other.id_ = -1;
  }
  void operator ==(Task&& other) {
    printf("operator ==(Task&& other), other.id=%d\n", other.id_);
    id_ = other.id_;
    other.id_ = -1;
  }
  
  int Id() const { return id_; }
private:
  int id_;
};

TEST(BlockingQueueTest, Push) {
  BlockingQueue<Task> queue;
  ASSERT_TRUE(queue.Empty());
  
  int id = 1;
  
  std::thread consumer_thread([&]() {
    printf("[consumer] take task\n");
    Task got = queue.Take(); // `take` always return a copy
    printf("[consumer] /take task, id=%d\n", got.Id());
    ASSERT_EQ(got.Id(), id);
  });
  
  std::thread producer_thread([&]() {
    printf("[producer] create task, id=%d\n", id);
    Task task(id);
    printf("[producer] push task, id=%d\n", task.Id());
    queue.Push(std::move(task));
    ASSERT_EQ(task.Id(), -1); // moved, the other has been reset
    printf("[producer] /push task, id=%d\n", task.Id());
    //queue.Push(task);
  });
  
  producer_thread.join();
  consumer_thread.join();
  ASSERT_TRUE(queue.Empty());
}
