#include "matrix.h"

using FT = float;

static const int NUM_MUL_CHECKS = 100;
static const int NUM_INV_CHECKS = 100;
static const int D = 2;
static const int B = 3;
static const float EPS = 0.1;

void runInverseTest() {
    for(int i = 0; i < NUM_INV_CHECKS;) {
        DiagMatrix<D, B, FT> m = genRandDiagFloatMatrix<D, B, FT>();
        std::cout << "running matmul check (" << i << ")...";
        CheckInverseResult cir = checkInverse(m, EPS);

        if (cir == CIRSuccess) {
            std::cout << "success!\n";
            i++;
        }
        else if (cir == CIRFail) {
            std::cout << "failed";
        }
        else {
            std::cout << "non-invertible, retrying\n";
        }

    }
}

void displayInverses() {
    while(1) {
        DiagMatrix<D, B, FT> m = genRandDiagFloatMatrix<D, B, FT>();
        bool success;
        RawMatrix<D, B, FT> raw = invRawMatrix<D, B, FT>(mkRawMatrix<D, B, FT>(m), success);
        if (!success) continue;

        std::cout << "====\n";
        std::cout << "MATRIX:\n";
        printDiag(m);
        std::cout << "INVERSE:\n";
        printRaw<D, B, FT>(raw);
        std::cout << "\n";

        char c;
        std::cout << "continue? [y/N]->";
        std::cin >> c;
        if (c == 'y') { continue; }
        break;
    }

}


int main() {
    srand(time(NULL));
    for(int i = 0; i < NUM_MUL_CHECKS; i++) {
        std::cout << "running matmul check (" << i << ")...";
        DiagMatrix<D, B, FT> m1 = genRandDiagFloatMatrix<D, B, FT>();
        DiagMatrix<D, B, FT> m2 = genRandDiagFloatMatrix<D, B, FT>();

        checkMatmulSameSize(m1, m2, EPS);
        std::cout << "success\n";

    }
    displayInverses();

    // runInverseTest();
    // std::cout << "MATMUL SUCCEEDS\n";
    // printSampleInverses();
    // std::cout << "INVERSE SUCCEEDS\n";
    return 0;
}
