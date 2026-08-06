#include "math-core/math-objects.hpp"

#include <stdexcept>
#include <complex>
#include <utility>
#include <cstdint>
#include <vector>


// Constructor Implementation
RealVector::RealVector(uint32_t dim) : v_data(dim, 0.0) {}
RealVector::RealVector(std::vector<double> input_data) : v_data(std::move(input_data)){}


// Constructor Implementation
ComplexVector::ComplexVector(uint32_t dim) : v_data(dim, {0.0, 0.0}) {}
ComplexVector::ComplexVector(std::vector<std::complex<double>> input_data) {
    if (input_data.size() < 2ull) {
        throw std::invalid_argument("A vector (order 1 tensor) must have at least 2 entries.");
    }
    v_data = std::move(input_data);
}


// Constructor Implementation
RealMatrix::RealMatrix(uint32_t rows, uint32_t cols) :
    m_rows(rows), m_cols(cols), m_data(rows * cols, 0.0) {
    if (m_rows < 2u && m_cols < 2u) {
        throw std::invalid_argument("A matrix (order 2 tensor) must have at least 2 entries.");
    }
}
RealMatrix::RealMatrix(uint32_t rows, uint32_t cols, std::vector<double> input_data) :
    m_rows(rows), m_cols(cols) {
    if (input_data.size() != rows * cols) {
        throw std::invalid_argument("Input array size does not match the provided rows and cols.");
    }
    m_data = std::move(input_data);
}


// Constructor Implementation
ComplexMatrix::ComplexMatrix(uint32_t rows, uint32_t cols) :
    m_rows(rows), m_cols(cols), m_data(rows * cols, {0.0, 0.0}) {
    if (m_rows < 2u && m_cols < 2u) {
        throw std::invalid_argument("A matrix (order 2 tensor) must have at least 2 entries.");
    }
}
ComplexMatrix::ComplexMatrix(uint32_t rows, uint32_t cols, std::vector<std::complex<double>> input_data) :
    m_rows(rows), m_cols(cols) {
    if (input_data.size() != rows * cols) {
        throw std::invalid_argument("Input array size does not match the provided rows and cols.");
    }
    m_data = std::move(input_data);
}