#include "storage-utilities/storage-utilities.hpp"

#include <iostream>
#include <filesystem>
#include <H5Cpp.h>

namespace fs = std::filesystem;

// Public
void SandboxSessionManager::save_whole_sandbox() const {
    fs::path s_filepath = saved_data_dir / active_filename;
    if (!fs::exists(s_filepath)) {
        std::cout << "[SANDBOX] Saving new sandbox session targeting: " << s_filepath.filename() << "\n";
    }
    else {std::cout << "[SANDBOX] Saving existing sandbox session from: " << s_filepath.filename() << "\n";}

    // TODO

    std::cout << "[SANDBOX] Successfully saved sandbox to: " << s_filepath.filename() << "\n";
}