#include "thread-pool.hpp"

#include <functional>
#include <thread>
#include <utility>
#include <mutex>

// Constructor Implementation
ThreadPool::ThreadPool(int thread_count) {
    for (int i = 0; i < thread_count; i++) {
        // Create and initialize a thread
        worker_threads.emplace_back([this] {
            // The loop that keeps every thread working
            while (true) {
                // FETCH A NEW TASK FROM QUEUE
                std::function<void()> task_TODO;
                {// Lock Scoping for automatic unique_lock release
                    // Prevent other threads from disturbing this task fetch
                    std::unique_lock<std::mutex> this_lock(this->thread_pool_lock);

                    // Sleep until a task arrives OR pool is ordered to shutdown
                    // !this->tasks_q.empty() prevents spurrious wake ups by OS
                    // while (!predicate()) {
                    //     1. Release lock
                    //     2. Sleep until notified
                    //     3. Wake up and re-acquire lock when notified
                    // }
                    this->broadcaster.wait(this_lock, [this] {
                        // true ->   WAKE = !empty(false) || shutdown(true)
                        // LOCKS this_lock to continue this locked scope
                        // false -> sleep = !empty(true) || shutdown(false)
                        // UNLOCKS this_lock to let other threads work
                        return !this->tasks_q.empty() || this->shutdown;
                    });

                    // End thread if there are no tasks left and ordered to shutdown
                    if (this->tasks_q.empty() && this->shutdown) {return;}

                    // Grab next task and take it out of the queue
                    task_TODO = std::move(this->tasks_q.front());
                    this->tasks_q.pop();
                }
                // EXECUTE THE TASK OUTSIDE THE LOCK
                task_TODO();
            }
        });
    }
}

// Destructor Implementation
ThreadPool::~ThreadPool() {
    {// Lock Scoping for automatic unique_lock release
        // Declare shutdown to all threads
        // lock wil FREEZE the wait at checking predicate()
        std::unique_lock<std::mutex> lock(thread_pool_lock);
        // By holding thread_pool_lock when modifying shutdown, 
        // the Main Thread guarantees that no worker thread is
        // mid-transition between checking the predicate and sleeping.
        shutdown = true;
    }
    // Wake up all sleeping threads for them to notice shutdown order
    broadcaster.notify_all();

    // Wait for all threads to finish their tasks before decomission
    for (std::thread& worker : worker_threads) {
        if (worker.joinable()) {worker.join();}
    }
}

// Public
void ThreadPool::EnqueueTask(std::function<void()> new_task) {
    {// Lock scoping for automatic unique_lock release
        // Queue a new task to the tasks_q
        std::unique_lock<std::mutex> lock(thread_pool_lock);
        tasks_q.push(std::move(new_task));
    }
    // Wake up a sleeping thread available for a task
    broadcaster.notify_one();
}