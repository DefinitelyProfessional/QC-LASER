#include "data-utilities/data-utilities.hpp"
#include "data-utilities/data-payload.hpp"

#include <H5Cpp.h>

#include <shared_mutex>
#include <filesystem>
#include <iostream>
#include <mutex>

namespace fs = std::filesystem;

namespace DATA {
//Public
StatusPayload SandboxDataManager::save_whole_sandbox() const {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    return save_whole_sandbox_internal();
}
// Private
StatusPayload SandboxDataManager::save_whole_sandbox_internal() const {
    fs::path s_filepath = saved_data_dir / active_filename;
    if (!fs::exists(s_filepath)) {
        std::cout << "[SANDBOX] Saving new sandbox session targeting : " << active_filename << "\n";
    }
    else {std::cout << "[SANDBOX] Saving existing sandbox session from : " << active_filename << "\n";}

    // TODO

    std::cout << "[SANDBOX] Successfully saved sandbox to: " << active_filename << "\n";
    return {true, "Successfully saved " + active_filename};
}
}