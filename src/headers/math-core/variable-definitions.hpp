#pragma once
#include <vector>
#include <complex>
#include <string>

// ========================================================================================================

// Versatile simple Vector. GenericVector(dim, data)
struct GenericVector {
    size_t v_dim;
    std::vector<std::complex<double>> v_data;

    // Default Constructor
    GenericVector() : v_dim(0), v_data() {}
    // Simple constructors
    GenericVector(size_t dim) : v_dim(dim), v_data(dim, {0.0, 0.0}) {}
    GenericVector(size_t dim, const std::vector<std::complex<double>>& input_data) :
        v_dim(dim) {
            if (input_data.size() != dim) {
                throw std::invalid_argument("Input array size do not match the provided dimensions.");
            }
            v_data = input_data;
        }
    // Inline accessor, 1st for read/write, 2nd for read only const&
    inline std::complex<double>& operator()(size_t i) {return v_data[i];}
    inline const std::complex<double>& operator()(size_t i) const {return v_data[i];}
    // Zero-copy raw pointer exposure for the UI buffer
    const std::complex<double>* raw_buffer() const {return v_data.data();}
};
// ========================================================================================================

// Versatile simple Matrix. GenericMatrix(rows, cols, data)
struct GenericMatrix {
    size_t m_rows;
    size_t m_cols;
    std::vector<std::complex<double>> m_data;

    // Default Constructor
    GenericMatrix() : m_rows(0), m_cols(0), m_data() {}
    // Simple constructors
    GenericMatrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols), m_data(rows * cols, {0.0, 0.0}) {}
    GenericMatrix(size_t rows, size_t cols, const std::vector<std::complex<double>>& input_data) :
        m_rows(rows), m_cols(cols) {
            if (input_data.size() != rows * cols) {
                throw std::invalid_argument("Input array size do not match the provided rows and cols.");
            }
            m_data = input_data;
        }
    // Inline accessor, 1st for read/write, 2nd for read only const&
    inline std::complex<double>& operator()(size_t i, size_t j) {return m_data[i * m_cols + j];}
    inline const std::complex<double>& operator()(size_t i, size_t j) const {return m_data[i * m_cols + j];}
    // Zero-copy raw pointer exposure for the UI buffer
    const std::complex<double>* raw_buffer() const {return m_data.data();}
};