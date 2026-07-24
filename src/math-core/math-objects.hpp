#pragma once

#include <complex>
#include <cstddef>
#include <vector>

// Versatile simple real number Vector. RealVector(dim, data)
struct RealVector {
    size_t v_dim{0};
    std::vector<double> v_data;

    // Constructors
    // RealVector() = default; // Trivial default constructor Inlined
    explicit RealVector(size_t dim);
    RealVector(size_t dim, std::vector<double> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline double& operator()(size_t i) { return v_data[i]; }
    inline const double& operator()(size_t i) const { return v_data[i]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const double* raw_buffer() const { return v_data.data(); }
};

// Versatile simple complex number Vector. ComplexVector(dim, data)
struct ComplexVector {
    size_t v_dim{0};
    std::vector<std::complex<double>> v_data;

    // Constructors
    // ComplexVector() = default; // Trivial default constructor Inlined
    explicit ComplexVector(size_t dim);
    ComplexVector(size_t dim, std::vector<std::complex<double>> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline std::complex<double>& operator()(size_t i) { return v_data[i]; }
    inline const std::complex<double>& operator()(size_t i) const { return v_data[i]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const std::complex<double>* raw_buffer() const { return v_data.data(); }
};

// Versatile simple real number Matrix. RealMatrix(rows, cols, data)
struct RealMatrix {
    size_t m_rows{0};
    size_t m_cols{0};
    std::vector<double> m_data;

    // Constructors
    // RealMatrix() = default; // Trivial default constructor (Inlined)
    RealMatrix(size_t rows, size_t cols);
    RealMatrix(size_t rows, size_t cols, std::vector<double> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline double& operator()(size_t i, size_t j) { return m_data[i * m_cols + j]; }
    inline const double& operator()(size_t i, size_t j) const { return m_data[i * m_cols + j]; }

    // Zero-copy raw pointer exposure for the UI buffer
    inline const double* raw_buffer() const {return m_data.data();}
};
// Versatile simple complex number Matrix. ComplexMatrix(rows, cols, data)
struct ComplexMatrix {
    size_t m_rows{0};
    size_t m_cols{0};
    std::vector<std::complex<double>> m_data;

    // Constructors
    // ComplexMatrix() = default; // Trivial default constructor (Inlined)
    ComplexMatrix(size_t rows, size_t cols);
    ComplexMatrix(size_t rows, size_t cols, std::vector<std::complex<double>> input_data);

    // Accessors inlined directly in header for zero function-call overhead
    inline std::complex<double>& operator()(size_t i, size_t j) { return m_data[i * m_cols + j]; }
    inline const std::complex<double>& operator()(size_t i, size_t j) const { return m_data[i * m_cols + j]; }

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