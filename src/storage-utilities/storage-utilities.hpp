#pragma once

#include "math-core/math-objects.hpp"

#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <variant>

// Type-safe union that can hold any of linear algebra objects
using LinAlgObject = std::variant<GenericMatrix, GenericVector>;

enum class LinAlgType : int {
    GenericMatrix = 0,
    GenericVector = 1
};

// Forward declare sqlite3
struct sqlite3;

// Manage Loading & Storing the Sandbox registry
class SandboxSessionManager {
private:
    std::filesystem::path saved_data_dir;
    std::string active_filename;
    std::unordered_map<std::string, LinAlgObject> s_registry;

    // Helper function to execute simple sqlite3
    static void execute_sql(sqlite3* db, const std::string& sql);
    
    // LOAD sandbox data from specified filename
    void load_sandbox();

public:
    // SandboxSessionManager Constructor
    explicit SandboxSessionManager(const std::filesystem::path& data_dir, const std::string& filename);

    // return the active sandbox filename string
    const std::string& get_active_filename() const {return active_filename;}

    // Trivial stuff
    const auto& get_all() const {return s_registry;}
    size_t count() const {return s_registry.size();}

    // Add or Overwrite an object in memory
    void add(const std::string& id, LinAlgObject obj) {
        s_registry.insert_or_assign(id, std::move(obj));
    }

    // Self explanatory removal by id key
    void remove(const std::string& id) {
        s_registry.erase(id);
    }

    // Safely retrieve a POINTER to the requested type, allowing in-place edits.
    // Returns nullptr if the ID doesn't exist OR if you ask for the wrong type.
    template<typename T>
    T* get_as(const std::string& id) {
        auto it = s_registry.find(id);
        if (it == s_registry.end()) {return nullptr;}
        
        // std::get_if safely checks the variant. 
        // If it holds a T, returns a pointer to it.
        return std::get_if<T>(&it->second);
    }

    // Dictionary key rename without copying heavy vector data
    void rename(const std::string& old_id, const std::string& new_id);

    // STORE sandbox data written back to filename
    void save_sandbox() const;

    // Save then delete previous sandbox, switch and load new sandbox
    void switch_sandbox(const std::string& new_filename);
};