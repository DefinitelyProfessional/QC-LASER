#pragma once

#include <complex>
#include <cstdint>
#include <vector>

/*
Unfortunately, for every new math object data type, 
there will be a LOT of source code repetition to implement -
BY HAND on several files, Here are the following checklist :

1. data-utilities/ :
- data-utilities.hpp register math objects in MathObjPool marked "[!!! SCALABLE !!!]"

2. data-utilities/HDF5-utilities/ : (almost every file in here basically)
- write-actions.hpp file-write implementation of individual math objects
- write-actions.cpp execution of writing every math objects to HDF5 file
- read-actions.cpp  load file data into data registry, bit tedious alert
- hdf5-utilities.hpp implement C++ datatypes to HDF5 native datatypes.
*/

// Versatile simple real number Vector. RealVector(dim, data)
struct RealVector {
    std::vector<double> v_data;

    // Constructors
    // Only pass dim, all entries default to 0.0, best for manual input
    explicit RealVector(uint32_t dim);
    // Only pass the vector data, moves mem ownership into this object
    explicit RealVector(std::vector<double> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline double& operator()(uint32_t i) {return v_data[i];}
    inline const double& operator()(uint32_t i) const {return v_data[i];}

    // Zero-copy raw pointer exposure for the UI buffer
    inline double* raw_buffer() {return v_data.data();}
    inline const double* raw_buffer() const {return v_data.data();}

    // Metadata text for UI
    static constexpr const char* name_txt = "Real Vector";
    static constexpr const char* def_txt = 
        "An element of an n-dimensional vector space over the field of real numbers.\n"
        "Represented as a vector of double-precision floats.";
};


// Versatile simple complex number Vector. ComplexVector(dim, data)
struct ComplexVector {
    std::vector<std::complex<double>> v_data;

    // Constructors
    // Only pass dim, all entries default to {0.0,0.0}, best for manual input
    explicit ComplexVector(uint32_t dim);
    // Only pass the vector data, moves mem ownership into this object
    explicit ComplexVector(std::vector<std::complex<double>> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline std::complex<double>& operator()(uint32_t i) {return v_data[i];}
    inline const std::complex<double>& operator()(uint32_t i) const {return v_data[i];}

    // Zero-copy raw pointer exposure for the UI buffer
    inline std::complex<double>* raw_buffer() {return v_data.data();}
    inline const std::complex<double>* raw_buffer() const {return v_data.data();}

    // Metadata text for UI, must be compatible for ImGui texts with no overhead
    static constexpr const char* name_txt = "Complex Vector";
    static constexpr const char* def_txt = 
        "An element of an n-dimensional vector space over the field of complex numbers.\n"
        "Represented as a vector of complex number double-precision floats.";
};


// Versatile simple real number Matrix. RealMatrix(rows, cols, data)
struct RealMatrix {
    uint32_t m_rows{0};
    uint32_t m_cols{0};
    std::vector<double> m_data;

    // Constructors
    // Only pass rows x cols, all entries default to 0.0, best for manual input
    RealMatrix(uint32_t rows, uint32_t cols);
    // Only pass the vector data, moves mem ownership into this object
    RealMatrix(uint32_t rows, uint32_t cols, std::vector<double> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline double& operator()(uint32_t i, uint32_t j) {return m_data[i * m_cols + j];}
    inline const double& operator()(uint32_t i, uint32_t j) const {return m_data[i * m_cols + j];}

    // Zero-copy raw pointer exposure for the UI buffer
    inline double* raw_buffer() {return m_data.data();}
    inline const double* raw_buffer() const {return m_data.data();}

    // Metadata text for UI, must be compatible for ImGui texts with no overhead
    static constexpr const char* name_txt = "Real Matrix";
    static constexpr const char* def_txt = 
        "Matrix over the field of real numbers.\n"
        "etc...";
};


// Versatile simple complex number Matrix. ComplexMatrix(rows, cols, data)
struct ComplexMatrix {
    uint32_t m_rows{0};
    uint32_t m_cols{0};
    std::vector<std::complex<double>> m_data;

    // Constructors
    // ComplexMatrix() = default; // Trivial default constructor (Inlined)
    ComplexMatrix(uint32_t rows, uint32_t cols);
    ComplexMatrix(uint32_t rows, uint32_t cols, std::vector<std::complex<double>> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline std::complex<double>& operator()(uint32_t i, uint32_t j) {return m_data[i * m_cols + j];}
    inline const std::complex<double>& operator()(uint32_t i, uint32_t j) const {return m_data[i * m_cols + j];}

    // Zero-copy raw pointer exposure for the UI buffer
    inline std::complex<double>* raw_buffer() {return m_data.data();}
    inline const std::complex<double>* raw_buffer() const {return m_data.data();}

    // Metadata text for UI, must be compatible for ImGui texts with no overhead
    static constexpr const char* name_txt = "Complex Matrix";
    static constexpr const char* def_txt = 
        "Matrix over the field of complex numbers.\n"
        "etc...";
};