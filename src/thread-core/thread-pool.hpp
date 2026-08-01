#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <queue>

namespace MULTI {
// Threadpool architecture for C++ execution concurrency
class ThreadPool {
private:
    // The Threads
    std::vector<std::thread> worker_threads;
    // The Task Queue
    std::queue<std::function<void()>> tasks_q;

    // Backbone tools for a ThreadPool
    std::mutex thread_pool_lock; // One thread at a time to get tasks
    std::condition_variable broadcaster; // Coordinate task fetch
    std::atomic<bool> shutdown = false; // signal shutdown sequence

    std::atomic<int> active_threads{0}; // Metrics for UI

public:
    // Constructor creates and nitializes worker threads
    explicit ThreadPool(int thread_count);

    // Destructor to handle resolving threads and graceful exit
    ~ThreadPool();

    // Assign new tasks for the worker threads
    void assign_task(std::function<void()> new_task);

    // To poll how many threads are active at a given time
    inline int get_active_threads() const {return active_threads.load(std::memory_order_relaxed);}
};
}