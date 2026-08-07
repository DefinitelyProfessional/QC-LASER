#pragma once

#include "data-utilities/data-payload.hpp"
#include "math-core/math-objects.hpp"

#include <boost/unordered/unordered_flat_map.hpp> 
#include <shared_mutex>
#include <string_view>
#include <filesystem>
#include <functional>
#include <optional>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <tuple>
#include <mutex>

namespace DATA {
// Manage Loading & Storing the Sandbox registry
class SandboxDataManager {
private:
    // Registry handle
    struct MathObjMap {
        uint32_t key_index;
        uint32_t obj_index;
        uint8_t type;
    };
    // Encapsulate obj_data and hash_key to sync with registry
    template<typename T> struct ObjEntry {
        uint64_t hash_key;
        T obj_data;
    };

    // ...Ts packs all types passed into TuplePool
    template<typename ...Ts>   
    // Ts... unpacks passed types as std::tuple<std::vector<ObjEntry<Ts>>
    using TuplePool = std::tuple<std::vector<ObjEntry<Ts>>...>;
    // REGISTER MATH OBJECTS HERE [!!!SCALABLE!!!]
    using MathObjPool = TuplePool<
        RealVector,
        ComplexVector,
        RealMatrix,
        ComplexMatrix
    >; 
    
    // CRAZY TEMPLATE METAPROGRAMMING FOR COMPILE TIME RECURSION FOR get_type<T>()
    template<typename T, typename Tuple> struct TupleIdx;

    // BASE CASE 
    template<typename T, typename ...Types>
    struct TupleIdx<T, std::tuple<T, Types...>> {
        static constexpr std::uint8_t idx = 0;
    };

    // RECURSIVE BODY
    template<typename T, typename U, typename ...Types>
    struct TupleIdx<T, std::tuple<U, Types...>> {
        static constexpr std::uint8_t idx = 1 + 
        TupleIdx<T, std::tuple<Types...>>::idx;
    };


    // Maps hashed key_strings to the location of the object in their pool
    boost::unordered_flat_map<uint64_t, MathObjMap> sandbox_registry;
    std::vector<std::string> key_str_pool; // Seperated for display for user
    MathObjPool obj_pool;
    
    // File variables
    std::filesystem::path saved_data_dir;
    std::string active_filename;
    // Read/Write locks for multithreading
    mutable std::shared_mutex sandbox_lock;


    // Compile-time routing : return corresponding data pool
    template<typename T> constexpr inline auto& get_pool() {
        return std::get<std::vector<ObjEntry<T>>>(obj_pool);
    }

    // Compile-time routing : return corresponding math object type 
    template<typename T> constexpr uint8_t get_type() {
        // Get std::vector<ObjEntry<T>> in MathObjPool
        return TupleIdx<std::vector<ObjEntry<T>>, MathObjPool>::idx;
    }

    // To help alongside swap_pop TMP + COMPILE TIME RECURSION ALERT
    template<uint8_t idx = 0, typename Func> 
    void exe_with_pool(uint8_t target_type_idx, Func&& func_name) {
        // Base case stop recursion if no corresponding type is found
        if constexpr (idx < std::tuple_size_v<MathObjPool>) {
            if (idx == target_type_idx) { // Found a matching pool
                // std::forward<T>(func_name) is the forwarded lambda function
                // (std::get<0>(obj_pool)) is the argument given to the lambda
                std::forward<Func>(func_name)(std::get<idx>(obj_pool));
            } else { // Recursively check next idx type (idx + 1)
                exe_with_pool<idx + 1>(target_type_idx, std::forward<Func>(func_name));
            }
        }
    }


    // Helper to manage delete objects from their pools
    template<typename T> void swap_pop(std::vector<ObjEntry<T>>& obj_pool, uint32_t selected_obj_idx, uint32_t selected_key_idx) {
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

    // Return the active sandbox filename string
    const std::string get_active_filename() const {
        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        return active_filename;
    }
    // Return the vector string of keys for display to the user
    const std::vector<std::string> get_key_str_pool() const {
        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        return key_str_pool;
    }
    // Return the amount of objects present in the registry
    size_t count() const {
        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        return sandbox_registry.size();
    }


    // Add an object to sandbox_registry and handle data
    template<typename T> StatusPayload add(std::string_view key, T obj) {
        uint64_t hash = get_hash_key(key);

        // Unique lock guarantees exclusive access to modify sandbox data
        std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);

        // Reject adding objects with the same key_str
        if (sandbox_registry.find(hash) != sandbox_registry.end()) {
            return {std::string(key) + " already exists.", false};
        }

        // Get corresponding pool and type
        auto& obj_pool = get_pool<T>();
        uint8_t type = get_type<T>();

        // get the index at the obj_pool
        uint32_t obj_index = obj_pool.size();
        // get the index at the key_str_pool
        uint32_t key_index = key_str_pool.size();
        // Store obj at corresponding obj_pool
        obj_pool.push_back({hash, std::move(obj)});
        // Store key at corresponding key_pool
        key_str_pool.push_back(std::string(key));

        // Register to sandbox_registry
        sandbox_registry[hash] = {key_index, obj_index,  type};
        return {"Added " + std::string(key), true};
    }


    // Get a hard copy of the object, registry keeps its original object untouched
    template<typename T> DataPayload<T> get_copy(std::string_view key) {
        uint64_t hash = get_hash_key(key);

        // Shared_lock enables other threads to read data but not write
        std::shared_lock<std::shared_mutex> read_lock(sandbox_lock);
        
        // Get map registry of specified hash
        auto it = sandbox_registry.find(hash);
        // If specified key and its object doesn't exist
        if (it == sandbox_registry.end()) {
            return {std::nullopt, std::string(key) + " already exists.", false};
        }
        // If specified type doesn't match the object's type
        if (it->second.type != get_type<T>()) {
            return {std::nullopt, std::string(key) + " object type mismatch.", false};
        }

        // Return hard copy, registry keeps the original
        return {get_pool<T>()[it->second.obj_index].obj_data, "Copied " + std::string(key), true};
    }


    // Move the object out of the registry without copy, registry no longer has the object
    template<typename T> DataPayload<T> get_move(std::string_view key) {
        uint64_t hash = get_hash_key(key);

        // Unique lock guarantees exclusive access to modify sandbox data
        std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
        
        // Get map registry of specified hash
        auto it = sandbox_registry.find(hash);
        // If specified key and its object doesn't exist
        if (it == sandbox_registry.end()) {
            return {std::nullopt, std::string(key) + " doesn't exist.", false};
        }
        MathObjMap selected_map = it->second;
        // If specified type doesn't match the object's type
        if (selected_map.type != get_type<T>()) {
            return {std::nullopt, std::string(key) + " object type mismatch.", false};
        }
        
        auto& obj_pool = get_pool<T>();

        // Moved the object out of the ObjEntry pool
        T moved_obj = std::move(obj_pool[selected_map.obj_index].obj_data);
        // Clean the registry of the empty husk
        swap_pop(obj_pool, selected_map.obj_index, selected_map.key_index);
        sandbox_registry.erase(it);

        // Return moved object, registry no longer has it
        return {moved_obj, "Moved " + std::string(key), true};
    }

    // Remove an object from sandbox_registry and handle delete [!!!SCALABLE!!!]
    StatusPayload remove(std::string_view key);

    // Dictionary key rename without copying heavy vector data [!!!SCALABLE!!!]
    StatusPayload rename(std::string_view old_key, std::string_view new_key);

    // STORE sandbox data written back to filename
    StatusPayload save_whole_sandbox() const;

    // Save then delete previous sandbox, switch and load new sandbox
    StatusPayload switch_whole_sandbox(const std::string_view new_target);
};
}