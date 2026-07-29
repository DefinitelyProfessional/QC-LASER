#include "data-utilities/data-utilities.hpp"
#include "data-utilities/data-payload.hpp"

#include <boost/unordered/unordered_flat_map.hpp> // IWYU pragma: export

#include <shared_mutex>
#include <string_view>
#include <filesystem>
#include <cstdint>
#include <string>
#include <mutex>

namespace fs = std::filesystem;

namespace DATA {
// Constructor Implementation
SandboxDataManager::SandboxDataManager(const fs::path& data_dir, const std::string_view filename) :
saved_data_dir(data_dir), active_filename(filename) {
    load_whole_sandbox();
}

// Public [!!!SCALABLE!!!]
StatusPayload SandboxDataManager::remove(std::string_view key) {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    
    uint64_t hash = get_hash_key(key);
    // Get map registry of specified hash
    auto it = sandbox_registry.find(hash);
    // Specified key has no match aka doesn't exist
    if (it == sandbox_registry.end()) {
        return {std::string(key) + " doesn't exist.", false};
    }

    MathObjMap selected_map = it->second;
    // Remove the object from its pool
    exe_with_pool(selected_map.type,
    [this, &selected_map](auto& pool){
        swap_pop(
            pool,
            selected_map.obj_index,
            selected_map.key_index
        );
    });

    // Finally remove the map registry
    sandbox_registry.erase(it);
    return {"Removed " + std::string(key), true};
}

// Public [!!!SCALABLE!!!]
StatusPayload SandboxDataManager::rename(std::string_view old_key, std::string_view new_key) {
    // Return if there is no change in key
    if (old_key == new_key) {return {"Why rename the same key?", false};}

    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);

    auto old_it = sandbox_registry.find(get_hash_key(old_key));
    // Check if old_key already exists 
    if (old_it == sandbox_registry.end()) {
        return {std::string(old_key) + " doesn't exist.", false};
    }
    // Check if new_key already exists
    uint64_t new_hash = get_hash_key(new_key);
    if (sandbox_registry.find(new_hash) != sandbox_registry.end()) {
        return {std::string(old_key) + " already exists (or hash collision).", false};
    }

    // Update key_str and registry hash
    MathObjMap old_map_data = old_it->second;
    key_str_pool[old_map_data.key_index] = std::string(new_key);
    
    // Update the old ObjEntry hash_key to have the new_hash of renamed key
    exe_with_pool(old_map_data.type, [&old_map_data, &new_hash](auto& pool){
        pool[old_map_data.obj_index].hash_key = new_hash;
    });

    sandbox_registry.erase(old_it);
    sandbox_registry.emplace(new_hash, old_map_data);
    return {std::string(old_key) + " renamed to " + std::string(new_key), true};
}

// Public
StatusPayload SandboxDataManager:: switch_whole_sandbox(std::string_view new_target) {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    // Store current data to disk
    StatusPayload status = save_whole_sandbox_internal();
    if (!status.success) {return status;} // err check

    // Re-target the path and read the new database
    active_filename = new_target;

    // Attempt loading
    return load_whole_sandbox_internal();
}
}