#pragma once

#include "data-utilities/data-payload.hpp"
#include "math-core/math-objects.hpp"

#include <boost/unordered/unordered_flat_map.hpp> // IWYU pragma: export
#include <shared_mutex>
#include <type_traits>
#include <string_view>
#include <filesystem>
#include <functional>
#include <optional>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <mutex>

struct MathObjMap {
    MathObjType type;
    uint32_t obj_index;
    uint32_t key_index;
};

// Helper for static_assert in template branches
template<class> inline constexpr bool always_false_v = false;

// Manage Loading & Storing the Sandbox registry
class SandboxDataManager {
private:
    // File variables
    std::filesystem::path saved_data_dir;
    std::string active_filename;

    // Tools for multithreading
    mutable std::shared_mutex sandbox_lock; // Read/Write Lock

    // Encapsulate obj_data and hash_key to sync with registry
    template<typename T> struct ObjEntry {
        uint64_t hash_key;
        T obj_data;
    };

    // sandbox_registry for tracking name strings by hash keys
    // connected to MathObj stored in dedicated vectors
    // in memory and tracked with MathObjMap
    boost::unordered_flat_map<uint64_t, MathObjMap> sandbox_registry;
    std::vector<std::string> key_str_pool; // display for user
    std::vector<ObjEntry<RealVector>> real_vector_pool;
    std::vector<ObjEntry<ComplexVector>> complex_vector_pool;
    std::vector<ObjEntry<RealMatrix>> real_matrix_pool;
    std::vector<ObjEntry<ComplexMatrix>> complex_matrix_pool;

    // Compile-time routing : return corresponding data pool [!!!SCALABLE!!!]
    template<typename T> auto& get_pool() {
        if constexpr (std::is_same_v<T, RealVector>) return real_vector_pool;
        else if constexpr (std::is_same_v<T, ComplexVector>) return complex_vector_pool;
        else if constexpr (std::is_same_v<T, RealMatrix>) return real_matrix_pool;
        else if constexpr (std::is_same_v<T, ComplexMatrix>) return complex_matrix_pool;
        else static_assert(always_false_v<T>, "Unsupported math object type.");
    }
    // Compile-time routing : return corresponding math object type [!!!SCALABLE!!!]
    template<typename T> constexpr MathObjType get_type() {
        if constexpr (std::is_same_v<T, RealVector>) return MathObjType::RealVector;
        else if constexpr (std::is_same_v<T, ComplexVector>) return MathObjType::ComplexVector;
        else if constexpr (std::is_same_v<T, RealMatrix>) return MathObjType::RealMatrix;
        else if constexpr (std::is_same_v<T, ComplexMatrix>) return MathObjType::ComplexMatrix;
        else static_assert(always_false_v<T>, "Unsupported math object type.");
    }
    // Compile-time routing : helper to manage delete objects from their pools
    template<typename T> inline void swap_pop(std::vector<ObjEntry<T>>& obj_pool, uint32_t selected_obj_idx, uint32_t selected_key_idx) {
        size_t last_obj_idx = obj_pool.size() - 1;
        size_t last_key_idx = key_str_pool.size() - 1;
        // Swap iff selected_index != last_index
        if (selected_obj_idx != last_obj_idx) {
            // Swap selected with last, note pool[idx] is ObjEntry which is movable
            obj_pool[selected_obj_idx] = std::move(obj_pool[last_obj_idx]);
            // Swap selected with last
            key_str_pool[selected_key_idx] = std::move(key_str_pool[last_key_idx]);
            // Effectively update MathObjMap obj_idx with moved ObjEntry's hash_key
            sandbox_registry[obj_pool[selected_obj_idx].hash_key].obj_index = selected_obj_idx;
            // Effectively update MathObjMap key_idx with moved string's hash_key
            uint64_t moved_string_hash = get_hash_key(key_str_pool[selected_key_idx]);
            sandbox_registry[moved_string_hash].key_index = selected_key_idx;
        }
        obj_pool.pop_back();
        key_str_pool.pop_back();
    }

    // Internal key_str hashing for memory efficient mapping 
    inline uint64_t get_hash_key(const std::string_view key_str) const {
        return std::hash<std::string_view>{}(key_str);
    }
    
    // LOAD sandbox data from specified filename
    StatusPayload load_whole_sandbox();

    // Internal implementation for working with shared mutex
    StatusPayload load_whole_sandbox_internal();
    StatusPayload save_whole_sandbox_internal() const;

public:
    // SandboxDataManager Constructor
    explicit SandboxDataManager(const std::filesystem::path& data_dir, const std::string_view filename);

    // return the active sandbox filename string
    const std::string get_active_filename() const {
        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        return active_filename;
    }
    // return the vector string of keys for display to the user
    const std::vector<std::string> get_key_str_pool() const {
        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        return key_str_pool;
    }
    // return the amount of objects present in the registry
    size_t count() const {
        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        return sandbox_registry.size();
    }


    // Add an object to sandbox_registry and handle data
    template<typename T> StatusPayload add(std::string key, T obj) {
        uint64_t hash = get_hash_key(key);

        // Unique lock guarantees exclusive access to modify sandbox data
        std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);

        // Reject adding objects with the same key_str
        if (sandbox_registry.find(hash) != sandbox_registry.end()) {
            return {false, key + " already exists."};
        }

        // Get corresponding pool and type
        auto& obj_pool = get_pool<T>();
        MathObjType type = get_type<T>();

        // get the index at the obj_pool
        uint32_t obj_index = obj_pool.size();
        // get the index at the key_str_pool
        uint32_t key_index = key_str_pool.size();
        // Store obj at corresponding obj_pool
        obj_pool.push_back({hash, std::move(obj)});
        // Store key at corresponding key_pool
        key_str_pool.push_back(key);

        // Register to sandbox_registry
        sandbox_registry[hash] = {type, obj_index, key_index};
        return {true, "Added " + key};
    }

    // Get a hard copy of the object, registry keeps its original object untouched
    template<typename T> DataPayload<T> get_copy(std::string key) {
        uint64_t hash = get_hash_key(key);

        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        
        // Get map registry of specified hash
        auto it = sandbox_registry.find(hash);
        // If specified type doesn't match the object's type
        if (it->second.type != get_type<T>()) {
            return {false, key + " object type mismatch.", std::nullopt};
        }
        // If specified key and its object doesn't exist
        if (it == sandbox_registry.end()) {
            return {false, key + " already exists.", std::nullopt};
        }

        // Return hard copy, registry keeps the original
        return {true, "Copied " + key, get_pool<T>()[it->second.obj_index].obj_data};
    }

    // Move the object out of the registry without copy, registry no longer has the object
    template<typename T> DataPayload<T> get_move(std::string key) {
        uint64_t hash = get_hash_key(key);

        // Unique lock guarantees exclusive access to modify sandbox data
        std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
        
        // Get map registry of specified hash
        auto it = sandbox_registry.find(hash);
        MathObjMap selected_map = it->second;

        // If specified type doesn't match the object's type
        if (selected_map.type != get_type<T>()) {
            return {false, key + " object type mismatch.", std::nullopt};
        }
        // If specified key and its object doesn't exist
        if (it == sandbox_registry.end()) {
            return {false, key + " doesn't exist.", std::nullopt};
        }
        
        auto& obj_pool = get_pool<T>();

        // Moved the object out of the ObjEntry pool
        T moved_obj = std::move(obj_pool[selected_map.obj_index].obj_data);
        // Clean the registry of the empty husk
        swap_pop(obj_pool, selected_map.obj_index, selected_map.key_index);
        sandbox_registry.erase(it);

        // Return moved object, registry no longer has it
        return {true, "Moved " + key, moved_obj};
    }

    // Remove an object from sandbox_registry and handle delete [!!!SCALABLE!!!]
    StatusPayload remove(std::string key);

    // Dictionary key rename without copying heavy vector data [!!!SCALABLE!!!]
    StatusPayload rename(std::string old_key, std::string new_key);

    // STORE sandbox data written back to filename
    StatusPayload save_whole_sandbox() const;

    // Save then delete previous sandbox, switch and load new sandbox
    StatusPayload switch_whole_sandbox(const std::string new_target);
};