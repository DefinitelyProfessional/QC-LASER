#include "data-utilities/HDF5-utilities/hdf5-utilities.hpp"
#include "math-core/math-objects.hpp"

#include <H5DataSpace.h>
#include <H5Attribute.h>
#include <H5CompType.h>
#include <H5PredType.h>
#include <H5Location.h>
#include <H5DataSet.h>
#include <H5Spublic.h>
#include <H5public.h>
#include <H5Group.h>
#include <H5Cpp.h>

#include <string>

void write_real_vector(H5::Group& group, const std::string& key, const RealVector& vec) {
    hsize_t dims[1] = {vec.v_data.size()};
    H5::DataSpace dspace(1, dims); // Define 1D data

    // Create dataset on group for storing RealVector
    H5::DataSet dset = group.createDataSet(
        std::string(key), H5::PredType::NATIVE_DOUBLE, dspace);

    // Commit the write to the dataset
    dset.write(vec.raw_buffer(), H5::PredType::NATIVE_DOUBLE);
}


void write_complex_vector(H5::Group& group, const std::string& key, const ComplexVector& vec) {
    hsize_t dims[1] = {vec.v_data.size()};
    H5::DataSpace dspace(1, dims); // Define 1D data

    // Create dataset on group for storing ComplexVector
    H5::CompType complex_type = get_complex_datatype();
    H5::DataSet dset = group.createDataSet(
        std::string(key), complex_type, dspace);

    // Commit the write to the dataset
    dset.write(vec.raw_buffer(), complex_type);
}


void write_real_matrix(H5::Group& group, const std::string& key, const RealMatrix& mat) {
    hsize_t dims[2] = { mat.m_rows, mat.m_cols };
    H5::DataSpace dspace(2, dims); // Define 2D data
    
    // Create dataset on group for storing RealMatrix
    H5::DataSet dset = group.createDataSet(
        key, H5::PredType::NATIVE_DOUBLE, dspace);
    
    // Commit the write to the dataset (Row-Major 2D order)
    dset.write(mat.raw_buffer(), H5::PredType::NATIVE_DOUBLE);
    
    // SCALAR dataspace for single-value attributes
    H5::DataSpace attr_space(H5S_SCALAR);
    
    // Attach m_rows as an attribute
    H5::Attribute rows_attr = dset.createAttribute("m_rows", H5::PredType::NATIVE_UINT32, attr_space);
    rows_attr.write(H5::PredType::NATIVE_UINT32, &mat.m_rows);
    
    // Attach m_cols as an attribute
    H5::Attribute cols_attr = dset.createAttribute("m_cols", H5::PredType::NATIVE_UINT32, attr_space);
    cols_attr.write(H5::PredType::NATIVE_UINT32, &mat.m_cols);
}


void write_complex_matrix(H5::Group& group, const std::string& key, const ComplexMatrix& mat) {
    hsize_t dims[2] = { mat.m_rows, mat.m_cols };
    H5::DataSpace dspace(2, dims); // Define 2D data
    
    // Create dataset on group for storing ComplexMatrix
    H5::CompType complex_type = get_complex_datatype();
    H5::DataSet dset = group.createDataSet(
        key, complex_type, dspace);
    
    // Commit the write to the dataset (Row-Major 2D order)
    dset.write(mat.raw_buffer(), complex_type);
    
    // SCALAR dataspace for single-value attributes
    H5::DataSpace attr_space(H5S_SCALAR);
    
    // Attach m_rows as an attribute
    H5::Attribute rows_attr = dset.createAttribute("m_rows", H5::PredType::NATIVE_UINT32, attr_space);
    rows_attr.write(H5::PredType::NATIVE_UINT32, &mat.m_rows);
    
    // Attach m_cols as an attribute
    H5::Attribute cols_attr = dset.createAttribute("m_cols", H5::PredType::NATIVE_UINT32, attr_space);
    cols_attr.write(H5::PredType::NATIVE_UINT32, &mat.m_cols);
}