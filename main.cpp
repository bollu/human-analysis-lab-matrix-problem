#include "matrix.h"

using FT = float;

static const int NUM_MUL_CHECKS = 100;
static const int NUM_INV_CHECKS = 100;
static const int D = 5;
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

// sanity check that the two matrices are inverses of each other
// by multiplying them and checking that the result is almost-identity.
template<int D, int B, typename T>
void sanityCheckInverse(RawMatrix<D, B, FT> m1, RawMatrix<D, B, FT> m2, float eps) {
    RawMatrix<D, B, FT> mul = mulRawMatrix<D, B, T>(m1, m2);

    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            const T val = mul[i][j];
            if (i == j) {
                // id[i][j] = 1
                assert(std::abs(1 - val) < eps);
            }
            else {
                // id[i][j] = 1
                assert(std::abs(val) < eps);
            }
        }
    }

    std::cout << "\nsanity check close-to-identity mat:\n";
    printRaw<D, B, T>(mul);
    std::cout << "\n";

}

void displayInverses() {
    while(1) {
        DiagMatrix<D, B, FT> d = genRandDiagFloatMatrix<D, B, FT>();
        RawMatrix<D, B, FT> raw = mkRawMatrix<D, B, FT>(d);
        bool success;
        RawMatrix<D, B, FT> inv = invRawMatrix<D, B, FT>(raw, success);

        if (!success) continue;

        sanityCheckInverse<D, B, FT>(raw, inv, EPS);

        std::cout << "====\n";
        std::cout << "MATRIX:\n";
        printDiag(d);
        std::cout << "INVERSE:\n";
        printRaw<D, B, FT>(inv);
        std::cout << "\n";

        char c;
        std::cout << "continue? [y/N]->";
        std::cin >> c;
        if (c == 'y') { continue; }
        return;
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
