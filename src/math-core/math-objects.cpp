#include "math-core/math-objects.hpp"
#include <stdexcept>

// Constructor Implementation
GenericVector::GenericVector(size_t dim) :
    v_dim(dim), v_data(dim, {0.0, 0.0}) {}
GenericVector::GenericVector(size_t dim, const std::vector<std::complex<double>>& input_data) :
    v_dim(dim) {
    if (input_data.size() != dim) {
        throw std::invalid_argument("Input array size does not match the provided dimension.");
    }
    v_data = input_data;
}

// Constructor Implementation
GenericMatrix::GenericMatrix(size_t rows, size_t cols) :
    m_rows(rows), m_cols(cols), m_data(rows * cols, {0.0, 0.0}) {}
GenericMatrix::GenericMatrix(size_t rows, size_t cols, const std::vector<std::complex<double>>& input_data) :
    m_rows(rows), m_cols(cols) {
    if (input_data.size() != rows * cols) {
        throw std::invalid_argument("Input array size does not match the provided rows and cols.");
    }
    m_data = input_data;
}