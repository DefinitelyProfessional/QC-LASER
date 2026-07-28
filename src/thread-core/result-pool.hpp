#pragma once

#include <functional>
#include <vector>
#include <mutex>

namespace MULTI {
// Thread output capture for main thread
class OutputPool {
private:
    // 2 separate results array buffers
    std::vector<std::function<void()>> results[2];
    // Backbone tools for a OutputPool
    std::mutex result_pool_lock;
    int write_idx = 0;
public:
    // Enqueue a new result task
    void enqueue(std::function<void()> result_task);

    // Execute all results and swap results with an empty one
    void execute_results();
};
}