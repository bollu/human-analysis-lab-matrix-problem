#include "matrix.h"
#include "cxxopts.hpp"

using FT = float;

static const int NUM_MUL_CHECKS = 100;
static const int NUM_INV_CHECKS = 1e5;
static const int D = 1;
static const int B = 2;
static const float EPS = 0.2;


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
        RawMatrix<D*B, FT> raw = mkRawMatrix<D,B, FT>(d);
        bool success  = false;
        RawMatrix<D*B, FT> inv = invRawMatrixCML<D*B, FT>(raw, success);

        if (!success) continue;

        std::cout << "====\n";
        std::cout << "MATRIX:\n";
        printDiag(d);
        std::cout << "INVERSE:\n";
        printRaw<D*B,FT>(inv);
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

    for(int i = 0; i < NUM_INV_CHECKS;) {
        RawMatrix<D*B, FT> m = genRandRawFloatMatrix<D*B, FT>();

        bool success = false;
        RawMatrix<D*B, FT> refinv = invRawMatrixCML<D*B, FT>(m, success);
        if (!success) continue;

        // sanity check that the m * inv == identity
        if(!checkInverseByMatmul<D*B, FT>(m, refinv, EPS)) {
            continue;
        }


        RawMatrix<D*B, FT> inv = invRawMatrixOurs<D*B, FT>(m, success);
        if (success != true) {
            std::cout << "CML inverse:\n";
            printRaw<D*B, FT>(refinv);
            std::cout << "\n";
            assert(success == true && "a matrix inversion that succeeded with CML did not succeed with ours");
        }


        const bool isEqual = isRawEqual<D*B, FT>(refinv,
                "CML (reference)",
                inv, 
                "ours", 
                EPS, LogLevel::LogOn);
        assert(isEqual && "matrices not equal!");
        i++;
    }
}

int main(int argc, char *argv[]) {


    bool multest = false;
    bool invtest = false;
    bool invtestrawmanual = false;
    bool mulexperimentdifferentsize = false;

    cxxopts::Options options(argv[0], 
            " - Command line options for the matrix tool");


    options
      .positional_help("[optional args]")
      .show_positional_help();

    options.add_options()
        ("multest", "run matmul tests", cxxopts::value<bool>(multest))
    ("invtest", "run matinv tests", cxxopts::value<bool>(invtest))
    ("invtestrawmanual", "check correctness of matinv for matraw from user input", cxxopts::value<bool>(invtestrawmanual))
    ("mulexperimentdifferentsize", 
     "check correctness of matinv for matraw from user input", 
     cxxopts::value<bool>(mulexperimentdifferentsize))
    ("help", "show help");

    srand(time(NULL));


    auto result = options.parse(argc, argv);
    if (result.count("help") || 
            (!multest && !invtest && !invtestrawmanual && !mulexperimentdifferentsize))
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
        //displayInverses();
        runInverseDiagTest();
        runInverseRawTest();
    }

    if (invtestrawmanual) {
        while(1) {
            RawMatrix<D*B, FT> m = inputRaw<D*B, FT>();

            bool refsuccess = false;
            // Note that this is code duplication, so reduce this
            // duplication!
            RawMatrix<D*B, FT> refinv = invRawMatrixCML<D*B, FT>(m, refsuccess);


            bool oursuccess = false;
            RawMatrix<D*B, FT> ourinv = invRawMatrixOurs<D*B, FT>(m, oursuccess);
            assert(refsuccess == oursuccess && "Somehow, our successes are different!");

            if (!oursuccess) {
                assert(!refsuccess);
                std::cout << "\n* matrix is not invertible, as agreed upon by CML and us.\n";
                continue;
            }


            std::cout << "REFERENCE INVERSE:\n";
            printRaw<D*B, FT>(refinv);

            std::cout << "OUR INVERSE:\n";
            printRaw<D*B, FT>(ourinv);



            const bool isEqual = isRawEqual<D*B, FT>(refinv,
                    "CML (reference)",
                    ourinv,
                    "ours", 
                    EPS, LogLevel::LogOn);
            assert(isEqual && "matrices not equal!");


            std::cout << "continue? [y/N]->";
            char c;
            std::cin >> c;

            if (c == 'y') { continue; }
            break;

        }
    }

    // Flag added to see what the output of multiplying matrices of the form
    // (B1, D1) * (B2, D2) is.
    /*
    if (mulexperimentdifferentsize) {
            DiagMatrix<6, 2, FT> m1 = genRandDiagFloatMatrix<6, 2, FT>();
            DiagMatrix<4, 4, FT> m2 = genRandDiagFloatMatrix<4, 4, FT>();

            RawMatrix<12, 1, FT> m3 = 
                mulRawMatrix<4, 4, FT>(
                        mkRawMatrix<6, 2, FT>(m1),
                        mkRawMatrix<4, 4, FT>(m2));

            std::cout << "m1:\n";
            printDiag(m1);
            std::cout << "\nm2:\n";
            printDiag(m2);
            std::cout << "m1 * m2 := \n";
            printRaw<12, 1, FT>(m3);
    }
    */

    // runInverseDiagTest();
    // std::cout << "MATMUL SUCCEEDS\n";
    // printSampleInverses();
    // std::cout << "INVERSE SUCCEEDS\n";
    return 0;
}
