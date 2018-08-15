#include "matrix.h"
#include "cxxopts.hpp"

using FT = float;

static const int NUM_MUL_CHECKS = 100;
static const int NUM_INV_CHECKS = 1e5;
static const int D = 1;
static const int B = 2;
static const float EPS = 0.2;

// sanity check that the two matrices are inverses of each other
// by multiplying them and checking that the result is almost-identity.
template<int D, int B, typename T>
void checkInverseByMatmul(RawMatrix<D, B, FT> m1, RawMatrix<D, B, FT> m2, float eps) {
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
}


// Check that our implementation of inverse matches what is expected.
void runInverseDiagTest() {
    std::cout << "\n\n** INVERSE TEST **\n";
    for(int i = 0; i < NUM_INV_CHECKS;) {
        DiagMatrix<D, B, FT> m = genRandDiagFloatMatrix<D, B, FT>();
        std::cout << "running matinv check (" << i << ")...";
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


// Display the inverses of our kind of matrices one by one
// so we can analyze what's going on.
void displayInverses() {
    while(1) {
        DiagMatrix<D, B, FT> d = genRandDiagFloatMatrix<D, B, FT>();
        RawMatrix<D, B, FT> raw = mkRawMatrix<D, B, FT>(d);
        bool success;
        RawMatrix<D, B, FT> inv = invRawMatrixCML<D, B, FT>(raw, success);

        if (!success) continue;

        // check that raw * inv == identity
        checkInverseByMatmul<D, B, FT>(raw, inv, EPS);

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

// Check that our implementation of inverseRaw works
void runInverseRawTest() {
    std::cout << "checking correctness of matrix inverse of rawMat\n";

    for(int i = 0; i < NUM_INV_CHECKS; i++) {
        RawMatrix<D, B, FT> m = genRandRawFloatMatrix<D, B, FT>();

        bool success;
        RawMatrix<D, B, FT> refinv = invRawMatrixCML<D, B, FT>(m, success);
        if (!success) continue;


        // sanity check that the m * inv == identity
        checkInverseByMatmul<D, B, FT>(m, refinv, EPS);

        RawMatrix<D, B, FT> inv = invRawMatrixOurs<D, B, FT>(m, success);
        if (success != true) {
            std::cout << "CML inverse:\n";
            printRaw<D, B, FT>(refinv);
            std::cout << "\n";
            assert(success == true && "a matrix inversion that succeeded with CML did not succeed with ours");
        }


        const bool isEqual = isRawEqual<D, B, FT>(refinv,
                "CML (reference)",
                inv, 
                "ours", 
                EPS, LogLevel::LogOn);
        assert(isEqual && "matrices not equal!");

        // sanity check that the m * inv == identity
        checkInverseByMatmul<D, B, FT>(m, inv, EPS);

    }
}

int main(int argc, char *argv[]) {


    bool multest = false;
    bool invtest = false;
    bool invtestrawmanual = false;

    cxxopts::Options options(argv[0], 
            " - Command line options for the matrix tool");


    options
      .positional_help("[optional args]")
      .show_positional_help();

    options.add_options()
        ("multest", "run matmul tests", cxxopts::value<bool>(multest))
    ("invtest", "run matinv tests", cxxopts::value<bool>(invtest))
    ("invtestrawmanual", "check correctness of matinv for matraw from user input", cxxopts::value<bool>(invtestrawmanual))
    ("help", "show help");

    srand(time(NULL));


    auto result = options.parse(argc, argv);
    if (result.count("help") || (!multest && !invtest && !invtestrawmanual))
    {
      std::cout << options.help({""}) << std::endl;
      exit(0);
    }

    if (multest) {
        for(int i = 0; i < NUM_MUL_CHECKS; i++) {
            std::cout << "running matmul check (" << i << ")...";
            DiagMatrix<D, B, FT> m1 = genRandDiagFloatMatrix<D, B, FT>();
            DiagMatrix<D, B, FT> m2 = genRandDiagFloatMatrix<D, B, FT>();

            checkMatmulSameSize(m1, m2, EPS);
            std::cout << "success\n";

        }
    }

    if (invtest) {
        runInverseRawTest();
        //displayInverses();
        runInverseDiagTest();
    }

    if (invtestrawmanual) {
        while(1) {
            RawMatrix<D, B, FT> m = inputRaw<D, B, FT>();

            bool refsuccess;
            // Note that this is code duplication, so reduce this
            // duplication!
            RawMatrix<D, B, FT> refinv = invRawMatrixCML<D, B, FT>(m, refsuccess);


            bool oursuccess;
            RawMatrix<D, B, FT> ourinv = invRawMatrixOurs<D, B, FT>(m, oursuccess);
            assert(refsuccess == oursuccess && "Somehow, our successes are different!");

            if (!oursuccess) {
                assert(!refsuccess);
                std::cout << "\n* matrix is not invertible, as agreed upon by CML and us.\n";
                continue;
            }


            std::cout << "REFERENCE INVERSE:\n";
            printRaw<D, B, FT>(refinv);

            std::cout << "OUR INVERSE:\n";
            printRaw<D, B, FT>(ourinv);



            const bool isEqual = isRawEqual<D, B, FT>(refinv,
                    "CML (reference)",
                    ourinv,
                    "ours", 
                    EPS, LogLevel::LogOn);
            assert(isEqual && "matrices not equal!");

            // sanity check that the m * inv == identity
            checkInverseByMatmul<D, B, FT>(m, ourinv, EPS);

            std::cout << "continue? [y/N]->";
            char c;
            std::cin >> c;

            if (c == 'y') { continue; }
            break;

        }
    }

    // runInverseDiagTest();
    // std::cout << "MATMUL SUCCEEDS\n";
    // printSampleInverses();
    // std::cout << "INVERSE SUCCEEDS\n";
    return 0;
}
