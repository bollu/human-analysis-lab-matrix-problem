#include <iostream>
#include <array>
// D = number of dimensions per diagonal
// B = number of blocks
template<int D, int B, typename T>
struct DiagMatrix {
    using Diag = std::array<T, D>;
    std::array<std::array<Diag, B>, B> blocks;
};

template<int D, int B, typename T>
DiagMatrix<D, B, T> mulDiag(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> d2) {
    // we can perform block wise multiplication
};

template<int D, int B, typename T>
DiagMatrix<D, B, T> invDiag(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> d2) {};

template<int D, int B, typename T>
using RawMatrix = std::array< std::array<T, D * B>, D * B>;

template<int D, int B, typename T>
RawMatrix<D, B, T> mkRawMatrix(DiagMatrix<D, B, T> m) {
    RawMatrix<D, B, T> raw;
    for(int r = 0; r < B; r++) {
        for(int c = 0; c < B; c++) {
            const typename DiagMatrix<D, B, T>::Diag diagelem = raw.blocks[r][c];
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

enum class LogLevel {
    LogOn = 1,
    LogOff = 0
};
template<int D, int B, typename T>
bool isRawEqual(RawMatrix<D, B, T> r1, RawMatrix<D, B, T> r2, LogLevel l) {
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            if (r1[i][j] != r2[i][j]) {

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


template<int D, int B, typename T>
void checkMatmul(DiagMatrix<D, B, T> d1, DiagMatrix<D, B, T> d2) {
    DiagMatrix<D, B, T> diag  = mulDiag(d1, d2);
    RawMatrix<D, B, T> raw = mulRawMatrix(mkRawMatrix(d1), mkRawMatrix(d2));
    RawMatrix<D, B, T> diag2raw = mkRawMatrix(diag);

    assert(isRawEqual(raw, diag2raw));
}



