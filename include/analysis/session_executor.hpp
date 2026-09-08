#pragma once

// A small, dependency-free serial executor used to keep VMMDLL/er2 calls
// off transport threads.  The queue is bounded so an MCP client cannot grow
// unbounded work while a DMA operation is in progress.

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <future>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

namespace unityexplorer::analysis {

class ExecutorStopped final : public std::runtime_error {
public:
  ExecutorStopped() : std::runtime_error("session executor is stopped") {}
};

class ExecutorBusy final : public std::runtime_error {
public:
  ExecutorBusy() : std::runtime_error("session executor queue is full") {}
};

class SessionExecutor final {
public:
  explicit SessionExecutor(std::size_t maxQueue = 64)
      : maxQueue_(maxQueue == 0 ? 1 : maxQueue), worker_([this] { Run(); }) {}

  SessionExecutor(const SessionExecutor &) = delete;
  SessionExecutor &operator=(const SessionExecutor &) = delete;

  ~SessionExecutor() { Stop(); }

  template <typename F>
  auto Submit(F &&function) -> std::future<std::invoke_result_t<F>> {
    using Result = std::invoke_result_t<F>;
    auto task = std::make_shared<std::packaged_task<Result()>>(
        std::forward<F>(function));
    std::future<Result> result = task->get_future();

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopping_) {
        throw ExecutorStopped();
      }
      if (queue_.size() >= maxQueue_) {
        throw ExecutorBusy();
      }
      queue_.emplace_back([task] { (*task)(); });
    }
    wake_.notify_one();
    return result;
  }

  // Non-throwing convenience for callers that only need enqueue status.
  bool Post(std::function<void()> function) {
    if (!function) {
      return false;
    }
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (stopping_ || queue_.size() >= maxQueue_) {
        return false;
      }
      queue_.emplace_back(std::move(function));
    }
    wake_.notify_one();
    return true;
  }

  // Stops accepting new work, waits for the active task, and drops queued
  // work.  A queued packaged_task is destroyed, which correctly reports a
  // broken promise to its future instead of executing after shutdown.
  void Stop() noexcept {
    bool joinWorker = false;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (!stopping_) {
        stopping_ = true;
        queue_.clear();
      }

      const bool calledFromWorker =
          worker_.joinable() && worker_.get_id() == std::this_thread::get_id();
      if (!calledFromWorker && worker_.joinable()) {
        if (!joinInProgress_) {
          joinInProgress_ = true;
          joinWorker = true;
        } else {
          // Another shutdown caller owns the join.  Wait for it so the
          // destructor cannot leave a joinable thread behind.
          wake_.wait(lock, [this] { return joined_; });
        }
      }
    }
    wake_.notify_all();
    if (joinWorker) {
      worker_.join();
      {
        std::lock_guard<std::mutex> lock(mutex_);
        joined_ = true;
      }
      wake_.notify_all();
    }
  }

  bool IsStopping() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return stopping_;
  }

  std::size_t Pending() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  bool Active() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_;
  }

  // Useful for code that may be called by an executor task.  Such code can
  // run a nested operation inline instead of waiting on its own queue.
  bool IsWorkerThread() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return worker_.joinable() && worker_.get_id() == std::this_thread::get_id();
  }

  std::size_t Capacity() const noexcept { return maxQueue_; }

private:
  void Run() noexcept {
    for (;;) {
      std::function<void()> function;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        wake_.wait(lock, [this] { return stopping_ || !queue_.empty(); });
        if (stopping_ && queue_.empty()) {
          return;
        }
        function = std::move(queue_.front());
        queue_.pop_front();
        active_ = true;
      }

      try {
        function();
      } catch (...) {
        // packaged_task captures exceptions in its future.  A raw Post task
        // has no observer, so an exception must never terminate the worker.
      }

      {
        std::lock_guard<std::mutex> lock(mutex_);
        active_ = false;
      }
    }
  }

  const std::size_t maxQueue_;
  mutable std::mutex mutex_;
  std::condition_variable wake_;
  std::deque<std::function<void()>> queue_;
  bool stopping_ = false;
  bool active_ = false;
  bool joinInProgress_ = false;
  bool joined_ = false;
  std::thread worker_;
};

} // namespace unityexplorer::analysis
