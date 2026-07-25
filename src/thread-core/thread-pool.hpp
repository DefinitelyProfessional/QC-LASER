#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <queue>

class ThreadPool {
private:
    // The Threads
    std::vector<std::thread> worker_threads;
    // The Task Queue
    std::queue<std::function<void()>> tasks_q;

    // Backbone tools for a ThreadPool
    std::mutex thread_pool_lock;
    std::condition_variable broadcaster;
    std::atomic<bool> shutdown = false;

public:
    // Constructor creates and nitializes worker threads
    explicit ThreadPool(int thread_count);

    // Destructor to handle resolving threads and graceful exit
    ~ThreadPool();

    // Assign new tasks for the worker threads
    void EnqueueTask(std::function<void()> new_task); 
};