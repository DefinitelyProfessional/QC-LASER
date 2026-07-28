#pragma once

#include <complex>
#include <cstdint>
#include <vector>

// Versatile simple real number Vector. RealVector(dim, data)
struct RealVector {
    std::vector<double> v_data;

    // Constructors
    // RealVector() = default; // Trivial default constructor Inlined
    explicit RealVector(uint32_t dim);
    RealVector(uint32_t dim, std::vector<double> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline double& operator()(uint32_t i) { return v_data[i]; }
    inline const double& operator()(uint32_t i) const { return v_data[i]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const double* raw_buffer() const { return v_data.data(); }
};

// Versatile simple complex number Vector. ComplexVector(dim, data)
struct ComplexVector {
    std::vector<std::complex<double>> v_data;

    // Constructors
    // ComplexVector() = default; // Trivial default constructor Inlined
    explicit ComplexVector(uint32_t dim);
    ComplexVector(uint32_t dim, std::vector<std::complex<double>> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline std::complex<double>& operator()(uint32_t i) { return v_data[i]; }
    inline const std::complex<double>& operator()(uint32_t i) const { return v_data[i]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const std::complex<double>* raw_buffer() const { return v_data.data(); }
};

// Versatile simple real number Matrix. RealMatrix(rows, cols, data)
struct RealMatrix {
    uint32_t m_rows{0};
    uint32_t m_cols{0};
    std::vector<double> m_data;

    // Constructors
    // RealMatrix() = default; // Trivial default constructor (Inlined)
    RealMatrix(uint32_t rows, uint32_t cols);
    RealMatrix(uint32_t rows, uint32_t cols, std::vector<double> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline double& operator()(uint32_t i, uint32_t j) { return m_data[i * m_cols + j]; }
    inline const double& operator()(uint32_t i, uint32_t j) const { return m_data[i * m_cols + j]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const double* raw_buffer() const {return m_data.data();}
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
    inline std::complex<double>& operator()(uint32_t i, uint32_t j) { return m_data[i * m_cols + j]; }
    inline const std::complex<double>& operator()(uint32_t i, uint32_t j) const { return m_data[i * m_cols + j]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const std::complex<double>* raw_buffer() const { return m_data.data(); }
};

// For templates
enum class MathObjType : uint8_t {
    RealVector = 0,
    ComplexVector = 1,
    RealMatrix = 2,
    ComplexMatrix = 3
};