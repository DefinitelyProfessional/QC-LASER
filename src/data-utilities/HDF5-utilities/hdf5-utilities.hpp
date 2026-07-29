#pragma once

#include <H5CompType.h>
#include <H5PredType.h>

#include <complex>

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