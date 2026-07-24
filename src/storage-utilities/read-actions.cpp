#include "storage-utilities/storage-utilities.hpp"

#include <filesystem>
#include <iostream>
#include <H5Cpp.h>

namespace fs = std::filesystem;

// Private
void SandboxSessionManager::load_whole_sandbox() {
    fs::path s_filepath = saved_data_dir / active_filename;
    if (!fs::exists(s_filepath)) {
        std::cout << "[SANDBOX] Creating new sandbox session targeting: " << s_filepath.filename() << "\n";
        return; // There is nothing to load, exit.
    }
    else {std::cout << "[SANDBOX] Loading existing sandbox session from: " << s_filepath.filename() << "\n";}

    // TODO

    std::cout << "[SANDBOX] Successfully loaded sandbox from: " << s_filepath.filename() << "\n";
}
