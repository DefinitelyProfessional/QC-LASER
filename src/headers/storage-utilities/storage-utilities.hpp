#pragma once
#include "math-core/variable-definitions.hpp"
#include <sqlite3.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

// Unique tags for each of your classes to guarantee safe recreation from disk
enum class LinalgType : int {
    GenericMatrix = 0,
    ClassicVector = 1
};

class SandboxSession {
private:
    fs::path m_filepath;
    std::unordered_map<std::string, std::shared_ptr<Matrix>> m_registry;

    // Helper to execute simple SQL commands safely
    static void execute_sql(sqlite3* db, const std::string& sql) {
        char* err_msg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string err = "SQL Error: ";
            if (err_msg) {err += err_msg; sqlite3_free(err_msg);}
            throw std::runtime_error(err);
        }
    }

public:
    // ====================================================================================================
    // THE LOAD PHASE (Constructor)
    // ====================================================================================================
    explicit SandboxSession(const std::string& filename) {
        // Ensure the saved-data directory exists in the portable distribution
        fs::path dir = fs::current_path() / "saved-data";
        if (!fs::exists(dir)) {fs::create_directories(dir);}
        // Create a safe guaranteed file path
        m_filepath = dir / filename;

        // If the file doesn't exist, start with empty memory and exit the load phase early.
        if (!fs::exists(m_filepath)) {
            std::cout << "Creating new sandbox session targeting: " << m_filepath.filename() << "\n";
            return; 
        }

        std::cout << "Loading existing sandbox session from: " << m_filepath.filename() << "\n";
        // db is the sqlite database object
        sqlite3* db;
        if (sqlite3_open(m_filepath.string().c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open SQLite database: " + m_filepath.string());
        }

        const char* query_sql = 
            "SELECT object_id, type_tag, rows, cols, is_unitary, is_hermitian, is_normalized, data_blob "
            "FROM linear_objects;";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                LinalgType tag = static_cast<LinalgType>(sqlite3_column_int(stmt, 1));
                size_t rows = static_cast<size_t>(sqlite3_column_int64(stmt, 2));
                size_t cols = static_cast<size_t>(sqlite3_column_int64(stmt, 3));

                bool is_unitary = sqlite3_column_int(stmt, 4) != 0;
                bool is_hermitian = sqlite3_column_int(stmt, 5) != 0;
                bool is_normalized = sqlite3_column_int(stmt, 6) != 0;

                const void* blob_data = sqlite3_column_blob(stmt, 7);
                int blob_bytes = sqlite3_column_bytes(stmt, 7);
                
                size_t total_elements = rows * cols;
                std::vector<std::complex<double>> data(total_elements);

                if (blob_bytes > 0 && blob_data) {
                    std::memcpy(data.data(), blob_data, blob_bytes);
                }

                // Polymorphically recreate the correct object based on the loaded tag
                std::shared_ptr<Matrix> obj;
                if (tag == LinalgType::ClassicVector) {
                    obj = std::make_shared<ClassicVector>(rows, data, id);
                } else {
                    obj = std::make_shared<Matrix>(rows, cols, data, id);
                }

                // Restore internal cached state (Requires 'friend class SandboxSession' in Matrix)
                obj->m_is_unitary = is_unitary;
                obj->m_is_hermitian = is_hermitian;
                obj->m_is_normalized = is_normalized;

                m_registry[id] = std::move(obj);
            }
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // ====================================================================================================
    // IN-MEMORY MANAGEMENT PHASE
    // ====================================================================================================
    
    // Add or Overwrite an object in memory
    void add(std::shared_ptr<Matrix> obj) {
        if (!obj) throw std::invalid_argument("Cannot add null object to sandbox.");
        m_registry[obj->get_id()] = std::move(obj);
    }

    // Delete an object from memory
    void remove(const std::string& id) {
        m_registry.erase(id);
    }

    // Retrieve an object, automatically downcasting it to the requested type (e.g. ClassicVector)
    template<typename T = Matrix>
    std::shared_ptr<T> get(const std::string& id) const {
        auto it = m_registry.find(id);
        if (it == m_registry.end()) {
            return nullptr; 
        }
        
        auto casted = std::dynamic_pointer_cast<T>(it->second);
        if (!casted) {
            throw std::runtime_error("Type mismatch: Object '" + id + "' is not of the requested type.");
        }
        return casted;
    }

    // Safely rename an object (updates both the dictionary key and the internal m_id)
    void rename(const std::string& old_id, const std::string& new_id) {
        auto it = m_registry.find(old_id);
        if (it != m_registry.end()) {
            auto obj = it->second;
            m_registry.erase(it);
            obj->set_id(new_id);
            m_registry[new_id] = std::move(obj);
        }
    }
    
    const auto& get_all() const { return m_registry; }
    size_t count() const { return m_registry.size(); }

    // ====================================================================================================
    // THE STORE PHASE
    // ====================================================================================================
    void save() const {
        sqlite3* db;
        if (sqlite3_open(m_filepath.string().c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open SQLite database for writing.");
        }

        try {
            execute_sql(db, "PRAGMA synchronous = NORMAL;");
            execute_sql(db, "PRAGMA journal_mode = WAL;"); // Write-Ahead Logging for speed
            
            // 1. Initialize schema if it's a completely new file
            execute_sql(db, 
                "CREATE TABLE IF NOT EXISTS linear_objects ("
                "    object_id TEXT PRIMARY KEY,"
                "    type_tag INTEGER,"
                "    rows INTEGER,"
                "    cols INTEGER,"
                "    is_unitary INTEGER,"
                "    is_hermitian INTEGER,"
                "    is_normalized INTEGER,"
                "    data_blob BLOB"
                ");"
            );

            execute_sql(db, "BEGIN TRANSACTION;");

            // 2. Wipe existing records to ensure perfect synchronization with current memory state
            execute_sql(db, "DELETE FROM linear_objects;");

            // 3. Bulk insert the current memory map
            const char* insert_sql = 
                "INSERT INTO linear_objects (object_id, type_tag, rows, cols, "
                "is_unitary, is_hermitian, is_normalized, data_blob) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
            
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);

            for (const auto& [id, obj] : m_registry) {
                LinalgType tag = LinalgType::GenericMatrix;
                if (dynamic_cast<const ClassicVector*>(obj.get()) != nullptr) {
                    tag = LinalgType::ClassicVector;
                }

                sqlite3_reset(stmt);
                
                sqlite3_bind_text(stmt, 1, obj->get_id().c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 2, static_cast<int>(tag));
                sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(obj->rows()));
                sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(obj->cols()));
                sqlite3_bind_int(stmt, 5, obj->m_is_unitary ? 1 : 0);
                sqlite3_bind_int(stmt, 6, obj->m_is_hermitian ? 1 : 0);
                sqlite3_bind_int(stmt, 7, obj->m_is_normalized ? 1 : 0);

                size_t byte_size = obj->rows() * obj->cols() * sizeof(std::complex<double>);
                sqlite3_bind_blob(stmt, 8, obj->raw_buffer(), static_cast<int>(byte_size), SQLITE_STATIC);

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
        std::cout << "Successfully saved sandbox to: " << m_filepath.filename() << "\n";
    }
};