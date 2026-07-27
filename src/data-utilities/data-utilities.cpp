#include "data-utilities/data-utilities.hpp"
#include "math-core/math-objects.hpp"

#include <boost/unordered/unordered_flat_map.hpp> // IWYU pragma: export

#include <shared_mutex>
#include <stdexcept>
#include <string_view>
#include <filesystem>
#include <cstdint>
#include <string>
#include <mutex>

namespace fs = std::filesystem;

// Constructor Implementation
SandboxManager::SandboxManager(const fs::path& data_dir, const std::string_view filename) :
    saved_data_dir(data_dir), active_filename(filename) {
        std::string err_buffer = "";
        load_whole_sandbox(err_buffer);
        if (!err_buffer.empty()) {throw std::runtime_error(err_buffer);}
    }

// Public
bool SandboxManager::remove(std::string key, std::string& err_buffer) {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    
    uint64_t hash = get_hash_key(key);
    // Get map registry of specified hash
    auto it = sandbox_registry.find(hash);
    // Specified key has no match aka doesn't exist
    if (it == sandbox_registry.end()) {
        err_buffer = std::string(key) + " doesn't exist.";
        return false;
    }

    MathObjMap selected_map = it->second;
    // Remove the object from its pool
    switch (selected_map.type) {
        case MathObjType::RealVector:
            swap_pop(real_vector_pool, selected_map.obj_index, selected_map.key_index);
            break;
        case MathObjType::ComplexVector:
            swap_pop(complex_vector_pool, selected_map.obj_index, selected_map.key_index);
            break;
        case MathObjType::RealMatrix:
            swap_pop(real_matrix_pool, selected_map.obj_index, selected_map.key_index);
            break;
        case MathObjType::ComplexMatrix:
            swap_pop(complex_matrix_pool, selected_map.obj_index, selected_map.key_index);
            break;
    }

    // Finally remove the map registry
    sandbox_registry.erase(it);
    return true;
}

// Public
bool SandboxManager::rename(std::string old_key, std::string new_key, std::string& err_buffer) {
    // Return if there is no change in key
    if (old_key == new_key) {return false;}

    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);

    // Check if old_key already exists 
    auto old_it = sandbox_registry.find(get_hash_key(old_key));
    if (old_it == sandbox_registry.end()) {
        err_buffer = std::string(old_key) + " doesn't exist.";
        return false;
    }
    // Check if new_key already exists
    uint64_t new_hash = get_hash_key(new_key);
    if (sandbox_registry.find(new_hash) != sandbox_registry.end()) {
        err_buffer = std::string(new_key) + " already exists (or hash collision).";
        return false;
    }
    // update key_str and registry hash
    MathObjMap old_map_data = old_it->second;
    key_str_pool[old_map_data.key_index] = std::string(new_key);
    sandbox_registry.erase(old_it);
    sandbox_registry.emplace(new_hash, old_map_data);
    return true;
}

// Public
void SandboxManager:: switch_whole_sandbox(std::string new_target, std::string& err_buffer) {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    // Store current data to disk
    save_whole_sandbox_internal(err_buffer);
    if (!err_buffer.empty()) {return;} // err check

    // Re-target the path and read the new database
    active_filename = new_target;

    // Attempt loading
    load_whole_sandbox_internal(err_buffer);
}