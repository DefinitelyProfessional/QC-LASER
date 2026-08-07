#include "data-utilities/HDF5-utilities/read-actions.hpp"

#include "data-utilities/HDF5-utilities/hdf5-utilities.hpp"
#include "data-utilities/data-utilities.hpp"
#include "data-utilities/data-payload.hpp"
// #include "math-core/math-objects.hpp"

#include <H5Exception.h>
#include <H5CompType.h>
#include <H5PredType.h>
#include <H5DataSet.h>
#include <H5Fpublic.h>
#include <H5public.h>
#include <H5Group.h>
#include <H5File.h>

#include <boost/unordered/unordered_flat_map.hpp> 
#include <shared_mutex>
#include <filesystem>
#include <exception>
#include <iostream>
#include <cstdint>
#include <utility>
#include <string>
#include <vector>
#include <mutex>

namespace fs = std::filesystem;

// Extra reserves to anticipate the addition of more data
constexpr hsize_t extra_reserves = 10; 

namespace DATA {
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
        std::cout << "\n[SANDBOX] Creating new sandbox session targeting : " << active_filename << "\n";
        return {"New Sandbox : " + active_filename, true}; // There is nothing to load, exit.
    } else {std::cout << "\n[SANDBOX] Loading existing sandbox session from : " << active_filename << "\n";}

    // Aggresive memory clearing to make room for loading new data
    sandbox_registry = boost::unordered_flat_map<uint64_t, MathObjMap>();
    key_str_pool = std::vector<std::string>();
    obj_pool = MathObjPool{};

    try {
        // Open with read-only access
        H5::H5File file(s_filepath.string(), H5F_ACC_RDONLY);

        // Open all groups [!!! SCALABLE !!!]
        H5::Group real_vec_group = file.openGroup("/RealVector");
        H5::Group complex_vec_group = file.openGroup("/ComplexVector");
        H5::Group real_mat_group = file.openGroup("/RealMatrix");
        H5::Group complex_mat_group = file.openGroup("/ComplexMatrix");

        // Count number of math objects in each datasets [!!! SCALABLE !!!]
        hsize_t rv_count = real_vec_group.getNumObjs();
        hsize_t cv_count = complex_vec_group.getNumObjs();
        hsize_t rm_count = real_mat_group.getNumObjs();
        hsize_t cm_count = complex_mat_group.getNumObjs();
        hsize_t total_count = rv_count + cv_count + rm_count + cm_count;
        // Reserve memory in heap with extra room for anticipating new data [!!! SCALABLE !!!]
        std::get<0>(obj_pool).reserve(rv_count + extra_reserves);
        std::get<1>(obj_pool).reserve(cv_count + extra_reserves);
        std::get<2>(obj_pool).reserve(rm_count + extra_reserves);
        std::get<3>(obj_pool).reserve(cm_count + extra_reserves);
        key_str_pool.reserve(total_count + extra_reserves);
        sandbox_registry.reserve(total_count + extra_reserves);

        // Handle adding math objects to the registry
        auto populate_registry = [this](const std::string& key, auto obj, uint8_t type, auto& target_pool) {
            uint64_t hash = get_hash_key(key);
            uint32_t obj_index = target_pool.size();
            uint32_t key_index = key_str_pool.size();
            
            target_pool.push_back({hash, std::move(obj)});
            key_str_pool.push_back(key);
            sandbox_registry[hash] = {key_index, obj_index, type};
        };

        // Extract RealVectors
        for (hsize_t i = 0; i < rv_count; ++i) {
            std::string key = real_vec_group.getObjnameByIdx(i);
            H5::DataSet dset = real_vec_group.openDataSet(key);
            
            // Query extent (number of elements) and allocate vector
            hssize_t elements = dset.getSpace().getSimpleExtentNpoints();
            RealVector vec(elements);
            
            // Read binary payload and inject into registry
            dset.read(vec.raw_buffer(), H5::PredType::NATIVE_DOUBLE);
            populate_registry(key, std::move(vec), 0, std::get<0>(obj_pool));
        }

        // Extract ComplexVectors
        H5::CompType complex_type = get_complex_datatype();
        for (hsize_t i = 0; i < cv_count; ++i) {
            std::string key = complex_vec_group.getObjnameByIdx(i);
            H5::DataSet dset = complex_vec_group.openDataSet(key);
            
            hssize_t elements = dset.getSpace().getSimpleExtentNpoints();
            ComplexVector vec(elements);
            
            dset.read(vec.raw_buffer(), complex_type);
            populate_registry(key, std::move(vec), 1, std::get<1>(obj_pool));
        }

        // Extract RealMatrices
        for (hsize_t i = 0; i < rm_count; ++i) {
            std::string key = real_mat_group.getObjnameByIdx(i);
            H5::DataSet dset = real_mat_group.openDataSet(key);
            
            // Extract dimensions from HDF5 Attributes
            uint32_t rows = 0, cols = 0;
            dset.openAttribute("m_rows").read(H5::PredType::NATIVE_UINT32, &rows);
            dset.openAttribute("m_cols").read(H5::PredType::NATIVE_UINT32, &cols);
            
            // Construct matrix and pull dset
            RealMatrix mat(rows, cols);
            dset.read(mat.raw_buffer(), H5::PredType::NATIVE_DOUBLE);
            populate_registry(key, std::move(mat), 2, std::get<2>(obj_pool));
        }

        // Extract ComplexMatrices
        for (hsize_t i = 0; i < cm_count; ++i) {
            std::string key = complex_mat_group.getObjnameByIdx(i);
            H5::DataSet dset = complex_mat_group.openDataSet(key);
            
            uint32_t rows = 0, cols = 0;
            dset.openAttribute("m_rows").read(H5::PredType::NATIVE_UINT32, &rows);
            dset.openAttribute("m_cols").read(H5::PredType::NATIVE_UINT32, &cols);
            
            ComplexMatrix mat(rows, cols);
            dset.read(mat.raw_buffer(), complex_type);
            populate_registry(key, std::move(mat), 3, std::get<3>(obj_pool));
        }

        file.close();


    // Handle HDF5 specific failures
    } catch (const H5::Exception& errH5) {
        std::string err_msg = errH5.getDetailMsg();
        
        // Critical Fallback
        sandbox_registry.clear();
        key_str_pool.clear();
        obj_pool = MathObjPool{};
        
        std::cerr << "[SANDBOX] HDF5 Load Failed: " << err_msg << "\n";
        return {"HDF5 Load Failed : " + err_msg, false};
    // Handle standard C++ exceptions
    } catch (const std::exception& err) {
        // Critical Fallback
        sandbox_registry.clear();
        key_str_pool.clear();
        obj_pool = MathObjPool{};
        
        std::cerr << "[SANDBOX] Standard Exception Failed: " << err.what() << "\n";
        return {"Standard Exception during save : " + std::string(err.what()), false};
    }

    
    std::cout << "[SANDBOX] Successfully loaded sandbox from : " << active_filename << "\n";
    return {"Successfully loaded " + active_filename, true};
}
}