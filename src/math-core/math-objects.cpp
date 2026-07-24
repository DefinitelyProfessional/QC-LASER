#include "math-core/math-objects.hpp"

#include <stdexcept>
#include <cstddef>
#include <complex>
#include <utility>
#include <vector>


// Constructor Implementation
RealVector::RealVector(size_t dim) :
    v_dim(dim), v_data(dim, 0.0) {}
RealVector::RealVector(size_t dim, std::vector<double> input_data) :
    v_dim(dim) {
    if (input_data.size() != dim) {
        throw std::invalid_argument("Input array size does not match the provided dimension.");
    }
    v_data = std::move(input_data);
}

// Constructor Implementation
ComplexVector::ComplexVector(size_t dim) :
    v_dim(dim), v_data(dim, {0.0, 0.0}) {}
ComplexVector::ComplexVector(size_t dim, std::vector<std::complex<double>> input_data) :
    v_dim(dim) {
    if (input_data.size() != dim) {
        throw std::invalid_argument("Input array size does not match the provided dimension.");
    }
    v_data = std::move(input_data);
}

// Constructor Implementation
RealMatrix::RealMatrix(size_t rows, size_t cols) :
    m_rows(rows), m_cols(cols), m_data(rows * cols, 0.0) {}
RealMatrix::RealMatrix(size_t rows, size_t cols, std::vector<double> input_data) :
    m_rows(rows), m_cols(cols) {
    if (input_data.size() != rows * cols) {
        throw std::invalid_argument("Input array size does not match the provided rows and cols.");
    }
    m_data = std::move(input_data);
}

// Constructor Implementation
ComplexMatrix::ComplexMatrix(size_t rows, size_t cols) :
    m_rows(rows), m_cols(cols), m_data(rows * cols, {0.0, 0.0}) {}
ComplexMatrix::ComplexMatrix(size_t rows, size_t cols, std::vector<std::complex<double>> input_data) :
    m_rows(rows), m_cols(cols) {
    if (input_data.size() != rows * cols) {
        throw std::invalid_argument("Input array size does not match the provided rows and cols.");
    }
    m_data = std::move(input_data);
}