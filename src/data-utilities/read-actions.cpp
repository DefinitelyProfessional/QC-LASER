#include "data-utilities/data-utilities.hpp"
#include "data-utilities/data-payload.hpp"
#include "math-core/math-objects.hpp"

#include <boost/unordered/unordered_flat_map.hpp> // IWYU pragma: export
#include <H5Cpp.h>

#include <shared_mutex>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <mutex>

namespace fs = std::filesystem;

// Private
StatusPayload SandboxDataManager::load_whole_sandbox() {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    return load_whole_sandbox_internal();
}
// Private
StatusPayload SandboxDataManager::load_whole_sandbox_internal() {
    fs::path s_filepath = saved_data_dir / active_filename;
    if (!fs::exists(s_filepath)) {
        std::cout << "[SANDBOX] Creating new sandbox session targeting : " << active_filename << "\n";
        return {true, "New Sandbox : " + active_filename}; // There is nothing to load, exit.
    }
    else {std::cout << "[SANDBOX] Loading existing sandbox session from : " << active_filename << "\n";}

    // Aggresive memory clearing to make room for loading new data
    sandbox_registry = boost::unordered_flat_map<uint64_t, MathObjMap>();
    key_str_pool = std::vector<std::string>();
    real_vector_pool = std::vector<ObjEntry<RealVector>>();
    complex_vector_pool = std::vector<ObjEntry<ComplexVector>>();
    real_matrix_pool = std::vector<ObjEntry<RealMatrix>>();
    complex_matrix_pool = std::vector<ObjEntry<ComplexMatrix>>();

    // TODO

    std::cout << "[SANDBOX] Successfully loaded sandbox from : " << active_filename << "\n";
    return {true, "Successfully loaded " + active_filename};
}
