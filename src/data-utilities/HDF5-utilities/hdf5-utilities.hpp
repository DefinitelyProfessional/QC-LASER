#pragma once

#include "math-core/math-objects.hpp"
#include <H5CompType.h>
#include <H5PredType.h>
#include <H5Group.h>
#include <H5Cpp.h>

#include <complex>
#include <string>

// Forward declare
struct RealVector;
struct ComplexVector;
struct RealMatrix;
struct ComplexMatrix;

inline H5::CompType get_complex_datatype() {
    // Define total byte size of the compund type 
    H5::CompType complex_type(sizeof(std::complex<double>));
    // Map real part to byte offset 0
    complex_type.insertMember("r", 0, H5::PredType::NATIVE_DOUBLE);
    // Map imaginary part to byte offset right after real
    complex_type.insertMember("i", sizeof(double), H5::PredType::NATIVE_DOUBLE);
    // return the constructed compound type
    return complex_type;
}

void write_real_vector(H5::Group& group, const std::string& key, const RealVector& vec);
void write_complex_vector(H5::Group& group, const std::string& key, const ComplexVector& vec);
void write_real_matrix(H5::Group& group, const std::string& key, const RealMatrix& mat);
void write_complex_matrix(H5::Group& group, const std::string& key, const ComplexMatrix& mat);