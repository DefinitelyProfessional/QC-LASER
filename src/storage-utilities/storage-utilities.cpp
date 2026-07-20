#include "storage-utilities/storage-utilities.hpp"

#include <sqlite3.h>
#include <iostream>
#include <cstring>
#include <stdexcept>

namespace fs = std::filesystem;

// Constructor Implementation
SandboxSessionManager::SandboxSessionManager(const fs::path& data_dir, const std::string& filename) :
    saved_data_dir(data_dir), active_filename(filename) {load_sandbox();}

// Private
void SandboxSessionManager::execute_sql(sqlite3* db, const std::string& sql) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::string err = "SQL Error: ";
        if (err_msg) {
            err += err_msg;
            sqlite3_free(err_msg);
        }
        throw std::runtime_error(err);
    }
}

// Private
void SandboxSessionManager::load_sandbox() {
    fs::path s_filepath = saved_data_dir / active_filename;
    if (!fs::exists(s_filepath)) {
        std::cout << "[SANDBOX] Creating new sandbox session targeting: " << s_filepath.filename() << "\n";
        return; // There is nothing to load, exit.
    }
    else {std::cout << "[SANDBOX] Loading existing sandbox session from: " << s_filepath.filename() << "\n";}
    
    sqlite3* db;
    if (sqlite3_open(s_filepath.string().c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("[SANDBOX] Failed to open SQLite database: " + s_filepath.string());
    }

    const char* query_sql = "SELECT object_id, type_tag, rows, cols, data_blob FROM linear_objects;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            LinAlgType tag = static_cast<LinAlgType>(sqlite3_column_int(stmt, 1));
            size_t rows = static_cast<size_t>(sqlite3_column_int64(stmt, 2));
            size_t cols = static_cast<size_t>(sqlite3_column_int64(stmt, 3));

            const void* blob_data = sqlite3_column_blob(stmt, 4);
            int blob_bytes = sqlite3_column_bytes(stmt, 4);
            
            size_t total_elements = rows * cols;
            std::vector<std::complex<double>> data(total_elements);

            if (blob_bytes > 0 && blob_data) {
                std::memcpy(data.data(), blob_data, blob_bytes);
            }

            // Emplace the correct struct directly into the variant map
            if (tag == LinAlgType::GenericVector) {
                s_registry.emplace(id, GenericVector(rows, data)); // rows represents v_dim here
            } else {
                s_registry.emplace(id, GenericMatrix(rows, cols, data));
            }
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::cout << "[SANDBOX] Successfully loaded sandbox from: " << s_filepath.filename() << "\n";
}

// Public
void SandboxSessionManager::rename(const std::string& old_id, const std::string& new_id) {
    if (old_id == new_id) {return;}
    auto node = s_registry.extract(old_id);
    if (!node.empty()) {
        node.key() = new_id;
        s_registry.insert(std::move(node));
    }
}

// Public
void SandboxSessionManager::save_sandbox() const {
    sqlite3* db;
    fs::path s_filepath = saved_data_dir / active_filename;
    if (sqlite3_open(s_filepath.string().c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open SQLite database for writing.");
    }

    try {
        execute_sql(db, "PRAGMA synchronous = NORMAL;");
        execute_sql(db, "PRAGMA journal_mode = WAL;");
        
        // Simplified Schema
        execute_sql(db, 
            "CREATE TABLE IF NOT EXISTS linear_objects ("
            "    object_id TEXT PRIMARY KEY,"
            "    type_tag INTEGER,"
            "    rows INTEGER,"
            "    cols INTEGER,"
            "    data_blob BLOB"
            ");"
        );

        execute_sql(db, "BEGIN TRANSACTION;");
        execute_sql(db, "DELETE FROM linear_objects;");

        const char* insert_sql = 
            "INSERT INTO linear_objects (object_id, type_tag, rows, cols, data_blob) "
            "VALUES (?, ?, ?, ?, ?);";
        
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);

        for (const auto& [id, obj_variant] : s_registry) {
            sqlite3_reset(stmt);
            sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

            // std::visit lets us dynamically inspect the variant and run type-specific code
            std::visit([&](const auto& obj) {
                using T = std::decay_t<decltype(obj)>;
                
                if constexpr (std::is_same_v<T, GenericMatrix>) {
                    sqlite3_bind_int(stmt, 2, static_cast<int>(LinAlgType::GenericMatrix));
                    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(obj.m_rows));
                    sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(obj.m_cols));
                    
                    size_t byte_size = obj.m_rows * obj.m_cols * sizeof(std::complex<double>);
                    sqlite3_bind_blob(stmt, 5, obj.raw_buffer(), static_cast<int>(byte_size), SQLITE_STATIC);
                
                } else if constexpr (std::is_same_v<T, GenericVector>) {
                    sqlite3_bind_int(stmt, 2, static_cast<int>(LinAlgType::GenericVector));
                    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(obj.v_dim));
                    sqlite3_bind_int64(stmt, 4, 1); // Force cols to 1 for vector mapping
                    
                    size_t byte_size = obj.v_dim * sizeof(std::complex<double>);
                    sqlite3_bind_blob(stmt, 5, obj.raw_buffer(), static_cast<int>(byte_size), SQLITE_STATIC);
                }
            }, obj_variant);

            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
        execute_sql(db, "COMMIT;");
        
    } catch (...) {
        execute_sql(db, "ROLLBACK;");
        sqlite3_close(db);
        throw;
    }

    sqlite3_close(db);
    std::cout << "[SANDBOX] Successfully saved sandbox to: " << s_filepath.filename() << "\n";
}

// Public
void SandboxSessionManager::switch_sandbox(const std::string& new_filename) {
    // Store current data to disk
    save_sandbox();

    // Kill the memory spike by assigning an empty map forces the immediate 
    // destruction of all heavy variants/vectors AND drops the bucket allocation.
    s_registry = std::unordered_map<std::string, LinAlgObject>(); 

    // Re-target the path and read the new database
    active_filename = new_filename;
    load_sandbox();
}