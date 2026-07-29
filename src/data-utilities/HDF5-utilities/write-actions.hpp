#pragma once

#include "data-utilities/HDF5-utilities/hdf5-utilities.hpp"
#include "math-core/math-objects.hpp"

#include <H5DataSpace.h>
#include <H5Attribute.h>
#include <H5CompType.h>
#include <H5DataSet.h>
#include <H5PredType.h>
#include <H5DataType.h>
#include <H5Spublic.h>
#include <H5public.h>
#include <H5Group.h>

#include <type_traits>
#include <string>

template<typename T> void write_math_object(
    H5::Group& group, const std::string& key, const T& obj) {
    // Compile-time routing for resolving HDF5 satatypes
    constexpr bool is_complex = std::is_same_v<T, ComplexVector> || std::is_same_v<T, ComplexMatrix>;
    constexpr bool is_matrix = std::is_same_v<T, RealMatrix> || std::is_same_v<T, ComplexMatrix>;

    // Resolve HDF5 DataType dynamically
    auto get_datatype = []() -> H5::DataType {
        if constexpr (is_complex) return get_complex_datatype();
        else return H5::PredType::NATIVE_DOUBLE;
    };
    H5::DataType datatype = get_datatype();

    // Matrix write logic (2D + attributes)
    if constexpr (is_matrix) {
        // Declare data dimensions as 2D
        hsize_t dims[2] = {obj.m_rows, obj.m_cols};
        H5::DataSpace dspace(2, dims);
        
        // Create the dataset to store the object's data
        H5::DataSet dset = group.createDataSet(key, datatype, dspace);
        dset.write(obj.raw_buffer(), datatype);
        
        // Scalar data to be the attribute for the dataset
        H5::DataSpace attr_space(H5S_SCALAR);
        H5::Attribute rows_attr = dset.createAttribute("m_rows", H5::PredType::NATIVE_UINT32, attr_space);
        rows_attr.write(H5::PredType::NATIVE_UINT32, &obj.m_rows);
        H5::Attribute cols_attr = dset.createAttribute("m_cols", H5::PredType::NATIVE_UINT32, attr_space);
        cols_attr.write(H5::PredType::NATIVE_UINT32, &obj.m_cols);
    } 
    // Vector write logic (1D no atributes)
    else {
        // Declare data dimensions as 1D
        hsize_t dims[1] = { static_cast<hsize_t>(obj.v_data.size()) };
        H5::DataSpace dspace(1, dims);
        
        // Create the dataset to store the object's data
        H5::DataSet dset = group.createDataSet(key, datatype, dspace);
        dset.write(obj.raw_buffer(), datatype);
    }
}