#define _GLIBCXX_DEBUG
#include <iostream>
#include <iomanip>
#include <array>
#include <complex>
#include <assert.h>
#define CML_IMPLEMENTATION
#include "cml.h"

static const int MAT_COLUMN_WIDTH = 7;
// D = number of dimensions per diagonal
// B = number of blocks
template<int D, int B, typename T>
struct DiagMatrix {
    using Diag = std::array<T, D>;
    std::array<std::array<Diag, B>, B> blocks;
};

template<int D, int B, typename T>
void printDiag(const DiagMatrix<D, B, T> &diag) {
    for(int i = 0; i < D*B; i++) {
        for(int j = 0; j < D*B; j++) {
            const int iblock = i / D;
            const int jblock = j / D;
            const int iinner = i % D;
            const int jinner = j % D;

            if(iinner == jinner) {
                std::cout << std::right << std::setw(MAT_COLUMN_WIDTH) << diag.blocks[iblock][jblock][iinner];
            }
            else {
                std::cout << std::right << std::setw(MAT_COLUMN_WIDTH) << "0";
            }
            std::cout <<" ";
        }
        std::cout << "\n";
    }
}



template<int D, int B, typename T> 
typename DiagMatrix<D, B, T>::Diag mkZeroDiag() {
    typename DiagMatrix<D, B, T>::Diag out;
    for(int i = 0; i < D; i++) { out[i] = 0; }
    return out;
}

// hm, this is interesting.
template<int D1, int B1, int D2, int B2, int X1, int X2, typename T>
DiagMatrix<X1, X2, T> mulDiagMatrixGeneral(DiagMatrix<D1, B1, T> m1, DiagMatrix<D2, B2, T> m2) {

};

template<int D, int B, typename T>
DiagMatrix<D, B, T> mulDiagMatrixSameSize(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> m2) {
    DiagMatrix<D, B, T> out;
    // we can perform block wise multiplication
    for(int i = 0; i < B; i++) {
        for(int j = 0; j < B; j++)  {
            // TODO: move this out into the constructor.
            out.blocks[i][j] = mkZeroDiag<D, B, T>();
            for(int k = 0; k < B; k++) {
                // multiply the diagonal element
                for(int d = 0; d < D; d++) {
                    out.blocks[i][j][d] += m1.blocks[i][k][d] * m2.blocks[k][j][d];
                }
            }
        }

    }

    return out;
};

template<int D, int B, typename T>
DiagMatrix<D, B, T> mkIdentityDiag() {
    DiagMatrix<D, B, T> out;
    for(int b = 0; b < B; b++) {
        for(int d = 0; d < D; d++) {
            out[b][b][d] = 1;
        }
    }

}



template<int D, int B, typename T>
DiagMatrix<D, B, T> invDiag(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> d2);

template<int D, int B, typename T>
using RawMatrix = std::array< std::array<T, D * B>, D * B>;


template<int D, int B, typename T>
void printRaw(const RawMatrix<D, B, T> &raw) {
    for(int i = 0; i < D*B; i++) {
        for(int j = 0; j < D*B; j++) {
            std::cout << std::setprecision(3) <<  std::left << std::setw(MAT_COLUMN_WIDTH) << raw[i][j] << " ";
        }
        std::cout << "\n";
    }
}



template<int D, int B, typename T>
RawMatrix<D, B, T> mkRawMatrix(DiagMatrix<D, B, T> m) {
    RawMatrix<D, B, T> raw;
    for(int i = 0; i < D*B; i++) {
        for(int j = 0; j < D*B; j++) {
            const int iblock = i / D;
            const int jblock = j / D;
            const int iinner = i % D;
            const int jinner = j % D;

            if(iinner == jinner) {
                raw[i][j] = m.blocks[iblock][jblock][iinner];
            }
            else {
               raw[i][j] = 0;
            }
        }
    }
    return raw;
}

template<int D, int B, typename T>
RawMatrix<D, B, T> mulRawMatrix(RawMatrix<D, B, T> m1, RawMatrix<D, B, T>m2) {
    RawMatrix<D, B, T> raw;
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            raw[i][j] = 0;
            for(int k = 0; k < D * B; k++) {
                raw[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    return raw;
}


// ouch, this cost is painful. Just switch to CML.
// TODO: switch to unique_ptr<, custom_dtor>;
template<int D, int B, typename T>
MATRIX *mkCMLFromRaw(RawMatrix<D, B, T> r) {
    MATRIX *cml = cml_new(D * B, D*B);
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            // TODO: look into cml_set_{row, col}
            cml_set(cml, i, j, r[i][j]);
        }
    }
    return cml;
}

template<int D, int B, typename T>
RawMatrix<D, B, T> invRawMatrix(RawMatrix<D, B, T> m, bool &success) {
    RawMatrix<D, B, T> out;
    MATRIX *cmlm = mkCMLFromRaw<D, B, T>(m);

    assert(cmlm != nullptr);
    success = cml_inverse(cmlm, NULL);
    if (!success) return out;

    // I am almost 100% sure I can memcpy() between these two, but just
    // to be safe, I hesitate to do that -- memory layouts and whatnot.
    // Let's get this running first, optimise later.
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            out[i][j] = cml_get(cmlm, i, j);
        }
    }

    return out;
}

enum class LogLevel {
    LogOn = 1,
    LogOff = 0
};
template<int D, int B, typename T>
bool isRawEqual(RawMatrix<D, B, T> r1, std::string r1Name, RawMatrix<D, B, T> r2, std::string r2Name, const T eps, LogLevel l) {
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            if (std::abs(r1[i][j] - r2[i][j]) > eps) {

                // we need to log
                if (l == LogLevel::LogOn) {
                    std::cout << "matrices differ at: (" << i << "," << j <<")\n";
                    std::cout << r1Name << "[" << i << "][" << j << "] = " << r1[i][j] << "\n";
                    std::cout << r2Name << "[" << i << "][" << j << "] = " << r2[i][j] << "\n";

                    std::cout<<r1Name << ":\n";
                    printRaw<D, B, T>(r1);

                    std::cout<<"\n"<<r2Name << ":\n";
                    printRaw<D, B, T>(r2);
                }
                

                return false;
            }
        }
    }
    return true;
}

// NOTE: actually use a PRNG, don't (rand() % mod) / SIZE, this will bias
// the results
template<int D, int B, typename FloatT>
DiagMatrix<D, B, FloatT> genRandDiagFloatMatrix(const int mod = 8, const int SIZE = 1) {
    DiagMatrix<D, B, FloatT> diag;
    for(int i = 0; i < B; i++) {
        for(int j = 0; j < B; j++) {
            for(int k = 0; k < D; k++) {
                const FloatT sign = rand() % 2 ? 1 : -1;
                const FloatT val = rand() % mod;
                diag.blocks[i][j][k] = (sign * val) / SIZE;
            }
        }
    }

    return diag;
}

// TODO: remove code duplication?
template<int D, int B, typename T>
void checkMatmulSameSize(DiagMatrix<D, B, T> d1, DiagMatrix<D, B, T> d2, const T eps) {
    DiagMatrix<D, B, T> diag  = mulDiagMatrixSameSize(d1, d2);
    std::cout<<"\nMULDIAG:\n";
    printDiag<D, B, T>(diag);
    std::cout<<"\n====\n";

    RawMatrix<D, B, T> raw = mulRawMatrix<D, B, T>(mkRawMatrix<D, B, T>(d1), 
            mkRawMatrix<D, B, T>(d2));
    
    std::cout<<"\nMULRAW:\n";
    printRaw<D, B, T>(raw);

    RawMatrix<D, B, T> diag2raw = mkRawMatrix(diag);

    const bool isEqual = isRawEqual<D, B, T>(raw, "raw", diag2raw, "diag", eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");
}

// I have no idea if the inverse of these matrices continues to be a
// diagonal matrix, so I'm going to run experiments and find out :)
// Yay to knowing how to program.
// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/matrix-inverse
// TODO: make this a Maybe
template<int D, int B, typename T>
RawMatrix<D, B, T> invDiagMatrix(DiagMatrix<D, B, T> m, bool &success) {
    RawMatrix<D, B, T> out;

    // (row, col) representation.
    // pivoting - look at blocks on the diagonal
    // We are pivoting *rows* around.
    for(int b = 0; b < B; b++) {
        for(int d = 0; d < D; d++) {
            // if we have a pivot element, continue
            if (m.blocks[b][b][d] != 0) continue;

            // we don't have a pivot element on this column, so look for the
            // row that has the largest element on *this column*.
            // Not all rows of the matrix are useful, only rows of other
            // blocks that have the same "d" will be active.
            int biggest_row = b;
            for(int r = 0; r < B; r++) {
                // In the rth block in the current(b)th collumn, look at the dth element.
                if (std::abs(m.blocks[r][b][d]) > std::abs(m.blocks[biggest_row][b][d])) {
                    biggest_row = r;
                }
            }

            // we now looked for the biggest row. If it is *equal* to the current
            // row, then we have all 0's on this dimension of the space,
            // so we're screwed, since this dimension is being sent into the null space.
            // Give up.
            if (biggest_row == b) {
                success = false;
                return out;
            }
            


        }

    }

    for(int c = 0; c < B * D; c++) {
        for(int r = 0; r < B * D; r++) {
            // scale everything down by the identity number
        }
    }

    return out;
};


enum CheckInverseResult {
    CIRSuccess,
    CIRFail,
    CIRNonInvertible
};

template<int D, int B, typename T>
CheckInverseResult checkInverse(DiagMatrix<D, B, T> d, const T eps) {

    bool success = false;
    RawMatrix<D, B, T> raw = invRawMatrix<D, B, T>(mkRawMatrix<D, B, T>(d), success);
    if (!success) return CIRNonInvertible;

    // we don't check for success here, so let's first check for success in the case of
    // raw
    bool diag_success = false;
    RawMatrix<D, B, T> diag_inverse  = invDiagMatrix(d, diag_success);

    assert(diag_success == true && "invDiag unable to invert matrix that invRaw can!");


    const bool isEqual = isRawEqual<D, B, T>(raw, "raw", diag_inverse, "diag", eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");

    if (isEqual) return CIRSuccess;
    return CIRFail;
}
