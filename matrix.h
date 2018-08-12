#include <iostream>
#include <iomanip>
#include <array>
#include <complex>
#include <assert.h>
#define CML_IMPLEMENTATION
#include "cml.h"

static const int MAT_COLUMN_WIDTH = 5;
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


template<int D, int B, typename T>
DiagMatrix<D, B, T> mulDiagMatrix(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> m2) {
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
            std::cout << std::right << std::setw(MAT_COLUMN_WIDTH) << raw[i][j] << " ";
        }
        std::cout << "\n";
    }
}



template<int D, int B, typename T>
RawMatrix<D, B, T> mkRawMatrix(DiagMatrix<D, B, T> m) {
    RawMatrix<D, B, T> raw;
    for(int r = 0; r < B; r++) {
        for(int c = 0; c < B; c++) {
            for(int ri = 0; ri < D; ri++) {
                for(int ci = 0; ci < D; ci++) {
                    if (ri == ci) {
                        raw[r * D + ri][c * D + ci] = m.blocks[r][c][ri];
                    }
                    else {
                        raw[r * D + ri][c * D + ci] = 0;
                    }
                }
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
RawMatrix<D, B, T> invRawMatrix(RawMatrix<D, B, T> m) {
    RawMatrix<D, B, T> out;
    MATRIX *cmlm = mkCMLFromRaw<D, B, T>(m);

    MATRIX *cmlout = nullptr;
    bool success = cml_inverse(cmlm, cmlout);
    assert (success && "matrix inverse failed!");

    // I am almost 100% sure I can memcpy() between these two, but just
    // to be safe, I hesitate to do that -- memory layouts and whatnot.
    // Let's get this running first, optimise later.
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            out[i][j] = cml_get(cmlout, i, j);
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
            for(int k = 0; k < B; k++) {
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
void checkMatmul(DiagMatrix<D, B, T> d1, DiagMatrix<D, B, T> d2, const T eps) {
    std::cout<< "\n# DIAG:\n";
    printDiag(d1);
    std::cout<< "\n# RAW:\n";
    auto diag_raw = mkRawMatrix<D, B, T>(d1);
    printRaw<D, B, T>(diag_raw);
    std::cout << "\n===\n";


    DiagMatrix<D, B, T> diag  = mulDiagMatrix(d1, d2);
    RawMatrix<D, B, T> raw = mulRawMatrix<D, B, T>(mkRawMatrix<D, B, T>(d1), 
            mkRawMatrix<D, B, T>(d2));
    RawMatrix<D, B, T> diag2raw = mkRawMatrix(diag);

    const bool isEqual = isRawEqual<D, B, T>(raw, "raw", diag2raw, "diag", eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");
}

// I have no idea if the inverse of these matrices continues to be a
// diagonal matrix, so I'm going to run experiments and find out :)
// Yay to knowing how to program
template<int D, int B, typename T>
RawMatrix<D, B, T> invDiagMatrix(DiagMatrix<D, B, T> m) {
    RawMatrix<D, B, T> out;
    return out;
};


template<int D, int B, typename T>
void checkInverse(DiagMatrix<D, B, T> d, const T eps) {
    // DiagMatrix<D, B, T> diag  = invDiagMatrix(d);
    RawMatrix<D, B, T> diag_inverse  = invDiagMatrix(d);

    RawMatrix<D, B, T> raw = invRawMatrix<D, B, T>(mkRawMatrix<D, B, T>(d));

    const bool isEqual = isRawEqual<D, B, T>(raw, "raw", diag_inverse, "diag", eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");
}
