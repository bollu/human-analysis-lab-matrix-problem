#include "matrix.h"

static const int NUM_MUL_CHECKS = 1000;
static const int D = 10;
static const int B = 10;


int main() {
    for(int i = 0; i < NUM_MUL_CHECKS; i++) {
        DiagMatrix<D, B, int> m1 = genRandDiagIntMatrix<D, B>();
        DiagMatrix<D, B, int> m2 = genRandDiagIntMatrix<D, B>();

        checkMatmul(m1, m2);

    }
    return 0;
}
