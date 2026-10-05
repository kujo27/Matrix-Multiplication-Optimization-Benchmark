#ifndef MATRIX_H
#define MATRIX_H

#include <initializer_list>
#include <vector>
#include <iostream>
#include <cassert>
#include <omp.h>
#include <algorithm>

template <typename T = double>
class Matrix {
private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<T> data_;

public:
    // Constructors
    Matrix() = delete;
    Matrix(std::initializer_list<std::initializer_list<T>> matrix);
    Matrix(std::size_t rows, std::size_t cols);

    T& operator[](std::size_t row, std::size_t col);
    const T& operator[](std::size_t row, std::size_t col) const;
    friend std::ostream& operator<<(std::ostream& out, const Matrix<T>& matrix) {
        for (std::size_t r = 0; r < matrix.rows_; ++r) {
            out << "[";
            for (std::size_t c = 0; c < matrix.cols_; ++c) {
                out << matrix[r, c];
                if (c + 1 < matrix.cols_) {
                   out << ", ";
                }
            }
            out << "]\n";
            }
            return out << "\n";
    }

    // Getters for debugging purposes
    int rows() const { return static_cast<int>(rows_); }
    int cols() const { return static_cast<int>(cols_); }
    int length() const { return static_cast<int>(rows_ * cols_); }

    void fillMatrix();
    Matrix<T> transpose_naive() const;
    Matrix<T> transpose_tiled(std::size_t tile_size = 32) const;
    Matrix<T> multiply_naive(const Matrix<T>& other) const;
    Matrix<T> multiply_reordered(const Matrix<T>& other) const;
    Matrix<T> multiply_transposed(const Matrix<T>& other) const;
    Matrix<T> multiply_tiled_reordered(const Matrix<T>& other, std::size_t tile_size = 32) const;

private:
    static std::vector<T> flatten(std::initializer_list<std::initializer_list<T>> matrix);
    void rightMatrixValidCheck(Matrix<T> other) const;
    void transposedMatrixValidCheck(Matrix<T> transposed) const;
};


template<typename T>
Matrix<T>::Matrix(std::initializer_list<std::initializer_list<T>> matrix) 
: rows_ {matrix.size()}, cols_ {matrix.begin()->size()}, data_ { flatten(matrix) } {}

template<typename T>
Matrix<T>::Matrix(std::size_t rows, std::size_t cols) 
: rows_ { rows }, cols_ { cols }, data_ (rows * cols) {}

template<typename T>
T& Matrix<T>::operator[](std::size_t row, std::size_t col) {
    assert(row < rows_);
    assert(col < cols_);
    return data_[col + row * cols_];
}

template<typename T>
const T& Matrix<T>::operator[](std::size_t row, std::size_t col) const {
    assert(row < rows_);
    assert(col < cols_);
    return data_[col + row * cols_];
}

template<typename T>
void  Matrix<T>::fillMatrix() {
    T value { 0 };
    for (std::size_t i { 0 }; i < rows_; ++i) {
        for (std::size_t j { 0 }; j < cols_; ++j) {
           (*this)[i, j] = value;
           ++value;
        }
    }
}

template<typename T>
Matrix<T> Matrix<T>::transpose_naive() const {
    Matrix<T> transpose { cols_, rows_ };

    for (std::size_t i {}; i < rows_; ++i) {
        for (std::size_t j {}; j < cols_; ++j) {
            transpose[j, i] = (*this)[i, j];
        }
    }

    return transpose;
}

template<typename T>
Matrix<T> Matrix<T>::transpose_tiled(std::size_t tile_size) const {
    Matrix<T> transpose { cols_, rows_ };

    for (std::size_t i = 0; i < rows_; i += tile_size) {
        for (std::size_t j = 0; j < cols_; j += tile_size) {
            
            std::size_t i_max = std::min(i + tile_size, rows_);
            std::size_t j_max = std::min(j + tile_size, cols_);

            for (std::size_t k = i; k < i_max; ++k) {
                for (std::size_t x = j; x < j_max; ++x) {
                    transpose[x, k] = (*this)[k, x];
                }
            }
        }
    }

    return transpose;
}

template<typename T>
Matrix<T> Matrix<T>::multiply_naive(const Matrix<T>& other) const {
    rightMatrixValidCheck(other);

    Matrix<T> product { rows_, other.cols_ };

    for (std::size_t i {}; i < rows_; ++i) {
        for (std::size_t j {}; j < other.cols_; ++j) {
            T sum { 0 };
            for (std::size_t k {}; k < cols_; ++k) {
                sum += (*this)[i, k] * other[k, j];
            }
            product[i, j] = sum;
        }
    }

    return product;
}

template<typename T>
Matrix<T> Matrix<T>::multiply_reordered(const Matrix<T>& other) const {
    rightMatrixValidCheck(other);

    Matrix<T> product { rows_, other.cols_ };

    for (std::size_t i {}; i < rows_; ++i) {
        for (std::size_t k {}; k < cols_; ++k) {
            for (std::size_t j {}; j < other.cols_; ++j) {
                product[i, j] += (*this)[i, k] * other[k, j];
            }
        }
    }

    return product;
}

template<typename T>
Matrix<T> Matrix<T>::multiply_transposed(const Matrix<T>& transposed) const {
    transposedMatrixValidCheck(transposed);

    Matrix<T> product { rows_, transposed.rows_ };

    for (std::size_t i {}; i < rows_; ++i) {
        for (std::size_t j {}; j < transposed.rows_; ++j) {
            T sum { 0 };
            for (std::size_t k {}; k < cols_; ++k) {
                sum += (*this)[i, k] * transposed[j, k];
            }
            product[i, j] = sum;
        }
    }

    return product;
}

template<typename T>
Matrix<T> Matrix<T>::multiply_tiled_reordered(const Matrix<T>& other, std::size_t tile_size) const {
    if (tile_size == 0) {
        throw std::invalid_argument("tile size must be greater than zero");
    }
    rightMatrixValidCheck(other);

    Matrix<T> product { rows_, other.cols_};

    #pragma omp parallel for collapse(2)
    for (std::size_t tile_i = 0; tile_i < rows_; tile_i += tile_size) {
        std::size_t i_end = std::min(tile_i + tile_size, rows_);
        for (std::size_t tile_j = 0; tile_j < other.cols_; tile_j += tile_size) {
            std::size_t j_end = std::min(tile_j + tile_size, product.cols_);
            for (std::size_t tile_k {}; tile_k < cols_; tile_k += tile_size) {
                std::size_t k_end = std::min(tile_k + tile_size, cols_);
                for (std::size_t i = { tile_i }; i < i_end; ++i) {
                    for (std::size_t k { tile_k }; k < k_end; ++k) {
                        for (std::size_t j { tile_j }; j < j_end; ++j) {
                            product[i, j] += (*this)[i, k] * other[k, j];
                        }
                    }
                }
            }
        }
    }

    return product;
}

template<typename T>
std::vector<T> Matrix<T>::flatten(std::initializer_list<std::initializer_list<T>> matrix) {
    std::vector<T> data {};

    for (const auto& row : matrix) {
        for (const auto& element : row) {
            data.push_back(element);
        }
    }
        
    return data;
} 

template<typename T>
void Matrix<T>::rightMatrixValidCheck(Matrix<T> other) const{
    if (cols_ != other.rows_) {
        throw std::invalid_argument(
            "Right matrix does not have same number of rows as left matrix's columns");
    }
}

template<typename T>
void Matrix<T>::transposedMatrixValidCheck(Matrix<T> transposed) const{
    if (cols_ != transposed.cols_) {
        throw std::invalid_argument(
            "Transposed matrix does not have same number of cols as left matrix's columns");
    }
}


#endif
