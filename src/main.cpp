#include "matrix.h"
#include <iostream>

int main() {
    // Demo here:

    Matrix<double> A { 3, 3 };
    A.fillMatrix();

    Matrix<double> B { 3, 3 };
    B.fillMatrix();

    std::cout << A.multiply_naive(B);
    
    return 0;
}
