/*********************************************************************************************************
 * All modification made by Cambricon Corporation: © 2020 Cambricon Corporation
 * All rights reserved.
 * All other contributions:
 * Copyright (C) 2014 by Vitaliy Vitsentiy
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Intel Corporation nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************/

#ifndef INFER_SERVER_UTIL_THREAD_POOL_H_
#define INFER_SERVER_UTIL_THREAD_POOL_H_

#include <glog/logging.h>

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

#include "threadsafe_queue.h"

namespace infer_server {

/**
 * @brief Task functor
 */
struct Task {
  /// Invoke the task function
  void operator()() {
    if (func) {
      (func)();
    } else {
      LOG(WARNING) << "No task function";
    }
  }

  /// Function to be invoked
  std::function<void()> func = nullptr;
  /// Task priority
  int64_t priority = 0;
  /**
   * @brief Construct a new Task object
   */
  Task() = default;
  /**
   * @brief Construct a new Task object
   *
   * @param f Function to be invoked
   * @param p Task priority
   */
  Task(std::function<void()> f, int64_t p) : func(f), priority(p) {}

  /**
   * @brief Function object for performing comparisons between tasks
   */
  struct Compare {
    /**
     * @brief Checks whether priority of the first task is less than the second
     *
     * @param lhs One task
     * @param rhs Another task
     * @retval true If lhs.priority < rhs.priority
     * @retval false Otherwise
     */
    bool operator()(const Task &lhs, const Task &rhs) { return lhs.priority < rhs.priority; }
  };
};

/**
 * @brief Thread pool to run user's functors with signature `ret func(params)`
 *
 * @tparam Q Type of underlying container to store the tasks
 */
template <typename Q = TSQueue<Task>, typename = std::enable_if<std::is_same<typename Q::value_type, Task>::value>>
class ThreadPool {
 public:
  /// Type of container
  using queue_type = Q;
  /// Type of task
  using task_type = typename std::enable_if<std::is_same<typename Q::value_type, Task>::value, Task>::type;

  /**
   * @brief Construct a new Thread Pool object
   *
   * @param th_init_func Init function invoked at start of each thread in pool
   * @param n_threads Number of threads
   */
  explicit ThreadPool(std::function<bool()> th_init_func, int n_threads = 0) : thread_init_func_(th_init_func) {
    if (n_threads) Resize(n_threads);
  }

  /**
   * @brief Destroy the Thread Pool object
   *
   * @note the destructor waits for all the functions in the queue to be finished
   */
  ~ThreadPool() { Stop(true); }

  /**
   * @brief Get the number of threads in the pool
   *
   * @return size_t Number of threads
   */
  size_t Size() const noexcept { return threads_.size(); }

  /**
   * @brief Get the number of idle threads in the pool
   *
   * @return int Number of idle threads
   */
  int IdleNumber() const noexcept { return n_waiting_; }

  /**
   * @brief Get the Thread at the specified index
   *
   * @param i The specified index
   * @return std::thread& A thread
   */
  std::thread &GetThread(int i) { return *threads_[i]; }

  /**
   * @brief Change the number of threads in the pool
   *
   * @warning Should be called from one thread, otherwise be careful to not interleave, also with this->stop()
   * @param n_threads Target number of threads
   */
  void Resize(size_t n_threads) noexcept;

  /**
   * @brief Wait for all computing threads to finish and stop all threads
   *
   * @param wait_all_task_done If wait_all_task_done == true, all the functions in the queue are run,
   *                           otherwise the queue is cleared without running the functions
   */
  void Stop(bool wait_all_task_done = false) noexcept;

  /**
   * @brief Empty the underlying queue
   */
  void ClearQueue() {
    task_type t;
    // empty the queue
    while (task_q_.TryPop(t)) {
    }
  }

  /**
   * @brief Pops a task
   *
   * @return task_type A task
   */
  task_type Pop() {
    task_type t;
    task_q_.TryPop(t);
    return t;
  }

  /**
   * @brief Run the user's function, returned value is templatized in future
   *
   * @tparam callable Type of callable object
   * @tparam arguments Type of arguments passed to callable
   * @param priority Task priority
   * @param f Callable object to be invoked
   * @param args Arguments passed to callable
   * @return std::future<typename std::result_of<callable(arguments...)>::type>
   *         A future that wraps the returned value of user's function,
   *         where the user can get the result and rethrow the catched exceptions
   */
  template <typename callable, typename... arguments>
  auto Push(int64_t priority, callable &&f, arguments &&... args)
      -> std::future<typename std::result_of<callable(arguments...)>::type> {
    VLOG(6) << "Sumbit one task to threadpool, priority: " << priority;
    VLOG(6) << "thread pool (idle/total): " << IdleNumber() << " / " << Size();
    auto pck = std::make_shared<std::packaged_task<typename std::result_of<callable(arguments...)>::type()>>(
        std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
    task_q_.Emplace([pck]() { (*pck)(); }, priority);
    cv_.notify_one();
    return pck->get_future();
  }

  /**
   * @brief Run the user's function without returned value
   *
   * @warning There's no future to wrap exceptions, therefore user should guarantee that task won't throw,
   *          otherwise the program may be corrupted
   * @tparam callable Type of callable object
   * @tparam arguments Type of arguments passed to callable
   * @param priority Task priority
   * @param f Callable object to be invoked
   * @param args Arguments passed to callable
   */
  template <typename callable, typename... arguments>
  void VoidPush(int64_t priority, callable &&f, arguments &&... args) {
    VLOG(6) << "Sumbit one task to threadpool, priority: " << priority;
    VLOG(6) << "thread pool (idle/total): " << IdleNumber() << " / " << Size();
    task_q_.Emplace(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...), priority);
    cv_.notify_one();
  }

 private:
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  ThreadPool &operator=(ThreadPool &&) = delete;

  void SetThread(int i) noexcept;

  std::vector<std::unique_ptr<std::thread>> threads_;
  std::vector<std::shared_ptr<std::atomic<bool>>> flags_;
  queue_type task_q_;
  std::atomic<bool> is_done_{false};
  std::atomic<bool> is_stop_{false};
  // how many threads are waiting
  std::atomic<int> n_waiting_{0};

  std::mutex mutex_;
  std::condition_variable cv_;

  std::function<bool()> thread_init_func_{nullptr};
};  // class ThreadPool

/// Alias of ThreadPool<TSQueue<Task>>
using EqualityThreadPool = ThreadPool<TSQueue<Task>>;
/// Alias of ThreadPool<ThreadSafeQueue<Task, std::priority_queue<Task, std::vector<Task>, Task::Compare>>>
using PriorityThreadPool =
    ThreadPool<ThreadSafeQueue<Task, std::priority_queue<Task, std::vector<Task>, Task::Compare>>>;

}  // namespace infer_server

#endif  // INFER_SERVER_UTIL_THREAD_POOL_H_
