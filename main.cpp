#include "matrix.h"

using FloatTy = float;

static const int NUM_MUL_CHECKS = 100;
static const int D = 10;
static const int B = 10;
static const float EPS = 0.1;


int main() {
    for(int i = 0; i < NUM_MUL_CHECKS; i++) {
        std::cout << "running matmul check (" << i << ")...";
        DiagMatrix<D, B, FloatTy> m1 = genRandDiagFloatMatrix<D, B, FloatTy>();
        DiagMatrix<D, B, FloatTy> m2 = genRandDiagFloatMatrix<D, B, FloatTy>();

        checkMatmul(m1, m2, EPS);
        std::cout << "success\n";

    }
    return 0;
}
