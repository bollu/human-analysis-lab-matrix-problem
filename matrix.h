#include <iostream>
#include <array>
#include <complex>
#include <assert.h>
#include "cml.h"
// D = number of dimensions per diagonal
// B = number of blocks
template<int D, int B, typename T>
struct DiagMatrix {
    using Diag = std::array<T, D>;
    std::array<std::array<Diag, B>, B> blocks;
};



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
DiagMatrix<D, B, T> invDiag(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> d2);

template<int D, int B, typename T>
using RawMatrix = std::array< std::array<T, D * B>, D * B>;

template<int D, int B, typename T>
RawMatrix<D, B, T> mkRawMatrix(DiagMatrix<D, B, T> m) {
    RawMatrix<D, B, T> raw;
    for(int r = 0; r < B; r++) {
        for(int c = 0; c < B; c++) {
            const typename DiagMatrix<D, B, T>::Diag diagelem = m.blocks[r][c];
            for(int d = 0; d < D; d++) {
                raw[B * r + d][B * c + d] = diagelem[d];
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
    MATRIX *cmlm = mkCMLFromRaw(m);

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
bool isRawEqual(RawMatrix<D, B, T> r1, RawMatrix<D, B, T> r2, const T eps, LogLevel l) {
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            if (std::abs(r1[i][j] - r2[i][j]) > eps) {

                // we need to log
                if (l == LogLevel::LogOn) {
                    std::cout << "matrices differ at: (" << i << "," << j <<")\n";
                    std::cout << "r1[" << i << "][" << j << "] = " << r1[i][j] << "\n";
                    std::cout << "r2[" << i << "][" << j << "] = " << r2[i][j] << "\n";
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
DiagMatrix<D, B, FloatT> genRandDiagFloatMatrix(const int mod = 8, const int SIZE = 2) {
    DiagMatrix<D, B, FloatT> diag;
    for(int i = 0; i < B; i++) {
        for(int j = 0; j < B; j++) {
            for(int k = 0; k < B; k++) {
                const int sign = rand() % 2 ? 1 : -1;
                const int val = rand() % mod;
                diag.blocks[i][j][k] = sign * val / SIZE;
            }
        }
    }

    return diag;
}


template<int D, int B, typename T>
void checkMatmul(DiagMatrix<D, B, T> d1, DiagMatrix<D, B, T> d2, const T eps) {
    DiagMatrix<D, B, T> diag  = mulDiagMatrix(d1, d2);
    RawMatrix<D, B, T> raw = mulRawMatrix<D, B, T>(mkRawMatrix<D, B, T>(d1), 
            mkRawMatrix<D, B, T>(d2));
    RawMatrix<D, B, T> diag2raw = mkRawMatrix(diag);

    const bool isEqual = isRawEqual<D, B, T>(raw, diag2raw, eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");
}

