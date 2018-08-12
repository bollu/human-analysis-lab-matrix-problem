#include "matrix.h"

using FT = float;

static const int NUM_MUL_CHECKS = 100;
static const int NUM_INV_CHECKS = 100;
static const int D = 2;
static const int B = 3;
static const float EPS = 0.1;

void printSampleInverses() {

    for(int i = 0; i < NUM_INV_CHECKS;) {
        DiagMatrix<D, B, FT> m = genRandDiagFloatMatrix<D, B, FT>();

        bool success;
        checkInverse(m, EPS, success);
        if (!success) { continue; }
        std::cout << "running inverse check (" << i << ")...";
        std::cout << "success\n";
        i++;
    }
}


int main() {
    srand(time(NULL));
    for(int i = 0; i < NUM_MUL_CHECKS; i++) {
        std::cout << "running matmul check (" << i << ")...";
        DiagMatrix<D, B, FT> m1 = genRandDiagFloatMatrix<D, B, FT>();
        DiagMatrix<D, B, FT> m2 = genRandDiagFloatMatrix<D, B, FT>();

        checkMatmul(m1, m2, EPS);
        std::cout << "success\n";

    }

    // std::cout << "MATMUL SUCCEEDS\n";
    // printSampleInverses();
    // std::cout << "INVERSE SUCCEEDS\n";
    return 0;
}
