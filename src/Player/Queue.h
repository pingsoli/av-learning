#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

// Thread-Safe Queue
template <typename T>
class Queue
{
public:
  Queue(std::size_t max_size = 20)
    : max_size_(max_size)
  {}
  ~Queue() {}

  Queue(const Queue&) = delete;
  Queue(Queue&&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue& operator=(Queue&&) = delete;

  void Push(T t) {
    std::unique_lock<std::mutex> lock(mutex_);
    full_cond_.wait(lock, [&] { return q_.size() < max_size_; });
    q_.push(t);
    empty_cond_.notify_one();
  }

  T Pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    empty_cond_.wait(lock, [&] { return !q_.empty(); });
    T copy = q_.front();
    q_.pop();
    full_cond_.notify_one();
    return copy;
  }

  std::size_t Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return q_.size();
  }

private:
  std::queue<T> q_;
  std::size_t max_size_;
  mutable std::mutex mutex_;
  std::condition_variable empty_cond_;
  std::condition_variable full_cond_;
};

#endif