#include "data-utilities/data-utilities.hpp"

#include "data-utilities/HDF5-utilities/write-actions.hpp"
#include "data-utilities/data-payload.hpp"

#include <H5Exception.h>
#include <H5Fpublic.h>
#include <H5Group.h>
#include <H5File.h>

#include <shared_mutex>
#include <filesystem>
#include <exception>
#include <iostream>
#include <string>
#include <mutex>

namespace fs = std::filesystem;

namespace DATA {
// Public
StatusPayload SandboxDataManager::save_whole_sandbox() const {
    // Unique lock guarantees exclusive access to modify sandbox data
    std::unique_lock<std::shared_mutex> write_lock(sandbox_lock);
    return save_whole_sandbox_internal();
}
// Private
StatusPayload SandboxDataManager::save_whole_sandbox_internal() const {
    fs::path s_filepath = saved_data_dir / active_filename;
    fs::path tmp_filepath = saved_data_dir / (active_filename + ".tmp");

    if (!fs::exists(s_filepath)) {
        std::cout << "\n[SANDBOX] Saving new sandbox session targeting : " << active_filename << "\n";
    } else {std::cout << "[SANDBOX] Saving existing sandbox session from : " << active_filename << "\n";}

    // Attempt to write into a temporary file
    try {
        // Overwrite mode
        H5::H5File file(tmp_filepath.string(), H5F_ACC_TRUNC);

        // Groups by math object class
        H5::Group real_vec_group = file.createGroup("/RealVector");
        H5::Group complex_vec_group = file.createGroup("/ComplexVector");
        H5::Group real_mat_group = file.createGroup("/RealMatrix");
        H5::Group complex_mat_group = file.createGroup("/ComplexMatrix");

        for (const auto& entry : std::get<0>(obj_pool)) {
            write_math_object(
                real_vec_group, 
                key_str_pool[sandbox_registry.at(entry.hash_key).key_index], 
                entry.obj_data
            );
        }
        for (const auto& entry : std::get<1>(obj_pool)) {
            write_math_object(
                complex_vec_group, 
                key_str_pool[sandbox_registry.at(entry.hash_key).key_index], 
                entry.obj_data
            );
        }
        for (const auto& entry : std::get<2>(obj_pool)) {
            write_math_object(
                real_mat_group, 
                key_str_pool[sandbox_registry.at(entry.hash_key).key_index], 
                entry.obj_data
            );
        }
        for (const auto& entry : std::get<3>(obj_pool)) {
            write_math_object(
                complex_mat_group, 
                key_str_pool[sandbox_registry.at(entry.hash_key).key_index], 
                entry.obj_data
            );
        }

        file.close(); 
    // Handle HDF5 specific failures
    } catch (const H5::Exception& errH5) {
        std::string err_msg = errH5.getDetailMsg();
        // Clean up the incomplete/corrupted temporary file
        if (fs::exists(tmp_filepath)) {fs::remove(tmp_filepath);}
        std::cout << "[SANDBOX] HDF5 Save Failed: " << err_msg << "\n";
        return {"HDF5 Save Failed : " + err_msg, false};
    // Handle standard C++ exceptions
    } catch (const std::exception& err) {
        // Clean up the incomplete/corrupted temporary file
        if (fs::exists(tmp_filepath)) {fs::remove(tmp_filepath);}
        std::cout << "[SANDBOX] Standard Exception Failed: " << err.what() << "\n";
        return {"Standard Exception during save : " + std::string(err.what()), false};
    }


    // Atomic Swap ONLY if no exceptions were thrown.
    try {
        // fs::rename automatically and atomically overwrites the target if it exists
        fs::rename(tmp_filepath, s_filepath);
    } catch (const fs::filesystem_error& e) {
        // Handle edge cases where the OS prevents the rename
        if (fs::exists(tmp_filepath)) {fs::remove(tmp_filepath);}
        return {"Filesystem swap failed : " + std::string(e.what()), false};
    }

    std::cout << "[SANDBOX] Successfully saved sandbox to: " << active_filename << "\n";
    return {"Successfully saved " + active_filename, true};
}
}