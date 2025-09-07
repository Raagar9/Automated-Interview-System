#include <iostream>
#include <Eigen/Dense> 

int main() {
    // Define two 2x2 matrices
    Eigen::Matrix2d matA;
    Eigen::Matrix2d matB;

    // Initialize the matrices with values
    matA << 1, 2,
            3, 4;

    matB << 5, 6,
            7, 8;

    // Perform matrix addition
    Eigen::Matrix2d matC = matA + matB;

    // Print the result
    std::cout << "Matrix A:\n" << matA << std::endl;
    std::cout << "Matrix B:\n" << matB << std::endl;
    std::cout << "Matrix C (A + B):\n" << matC << std::endl;

    return 0;
}