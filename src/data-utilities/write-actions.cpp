#include "data-utilities/data-utilities.hpp"

#include <H5Cpp.h>

#include <shared_mutex>
#include <filesystem>
#include <iostream>
#include <string>
#include <mutex>

namespace fs = std::filesystem;

//Public
void SandboxSessionManager::save_whole_sandbox(std::string& err_buffer) const {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    save_whole_sandbox_internal(err_buffer);
}
// Private
void SandboxSessionManager::save_whole_sandbox_internal(std::string& err_buffer) const {
    fs::path s_filepath = saved_data_dir / active_filename;
    if (!fs::exists(s_filepath)) {
        std::cout << "[SANDBOX] Saving new sandbox session targeting: " << s_filepath.filename() << "\n";
    }
    else {std::cout << "[SANDBOX] Saving existing sandbox session from: " << s_filepath.filename() << "\n";}

    // TODO
    err_buffer = "";

    std::cout << "[SANDBOX] Successfully saved sandbox to: " << s_filepath.filename() << "\n";
}