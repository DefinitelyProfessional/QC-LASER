#include "thread-core/result-pool.hpp"

#include <functional>
#include <utility>
#include <vector>
#include <mutex>

namespace MULTI {
void OutputPool::enqueue(std::function<void()> result_task) {
    std::lock_guard<std::mutex> lock(result_pool_lock);
    results[write_idx].push_back(std::move(result_task));
}

void OutputPool::execute_results() {
    int read_idx;
    {// Lock Scoping
        std::lock_guard<std::mutex> lock(result_pool_lock);
        read_idx = write_idx; // To read current array
        write_idx = 1 - write_idx; // Oscilate 0 to 1
    }
    for (auto& res : results[read_idx]) {
        res(); // execute the result like a task
    }
    // Clear the array that was read
    results[read_idx].clear();
}
}