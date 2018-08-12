#include <array>
// D = number of dimensions per diagonal
// B = number of blocks
template<int D, int B, typename T>
struct DiagMatrix {
    using Diag = std::array<T, D>;
    std::array<std::array<Diag, B>, B> blocks;
};

template<int D, int B, typename T>
DiagMatrix<D, B, T> mul(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> d2) {
    // we can perform block wise multiplication
};

template<int D, int B, typename T>
DiagMatrix<D, B, T> inv(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> d2) {};

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



