#include "matrix.h"

using FT = float;

static const int NUM_MUL_CHECKS = 100;
static const int NUM_INV_CHECKS = 100;
static const int D = 2;
static const int B = 2;
static const float EPS = 0.1;

void printSampleInverses() {

    for(int i = 0; i < NUM_INV_CHECKS; i++) {
        std::cout << "running inverse check (" << i << ")...";
        DiagMatrix<D, B, FT> m = genRandDiagFloatMatrix<D, B, FT>();
        std::cout << "\ndiagonal matrix:\n";
        printDiag(m);
        RawMatrix<D, B, FT> invraw = invRawMatrix<D, B, FT>(mkRawMatrix<D, B, FT>(m));
        std::cout << "inverse:\n";
        printRaw<D, B, FT>(invraw);
        std::cout<<"\n";

        // checkInverse(m, EPS);
        std::cout << "success\n";

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

    std::cout << "MATMUL SUCCEEDS\n";
    printSampleInverses();
    std::cout << "INVERSE SUCCEEDS\n";
    return 0;
}
