#define _GLIBCXX_DEBUG
#include <iostream>
#include <numeric>
#include <iomanip>
#include <array>
#include <complex>
#include <assert.h>
#define CML_IMPLEMENTATION
#include "cml.h"

//#define DEBUG 

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
                std::cout << std::right << std::setprecision(3) <<
                    std::setw(MAT_COLUMN_WIDTH) <<
                    diag.blocks[iblock][jblock][iinner];
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
constexpr int gcd(int A, int B) {
    if (A == 0) return B;
    return gcd(B % A, A);
}

constexpr int lcm(int A, int B) {
    return A * B / gcd(A, B);
}

// hm, this is interesting.
// TODO: I should have an enable_if D1 * B1 == D2 * B2, but that can wait.
template<int D1, int B1, int D2, int B2, typename T>
DiagMatrix<gcd(D1, D2), B1 * D1 / gcd(D1, D2), T> 
       mulDiagMatrixGeneral(DiagMatrix<D1, B1, T> m1, DiagMatrix<D2, B2, T> m2) {
 constexpr int BLOCK = gcd(B1, B2);
 constexpr int DIAG = B1 * D1 / BLOCK;

 DiagMatrix<DIAG, BLOCK, T> out;

 for(int i = 0; i < BLOCK; i++) {
     for(int j = 0; j < BLOCK; j++) {
         out.blocks[i][j] = mkZeroDiag<DIAG, BLOCK, T>();
         for(int k = 0; k < BLOCK; k++) {
             for(int d = 0; d < DIAG; d++) {
                    out.blocks[i][j][d] += m1.blocks[i][k][d] * m2.blocks[k][j][d];
             }
         }
     }
 }

 return out;

    

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


// B1 * D1 = B2 * D2
// What is the new (B, D) ?
// I believe it's going to be LCM.
// B1 / LCM(D1, D2) , LCM(D1, D2)
/*
template<int D1, int B1, int D2, int B2, typename T>
DiagMatrix<D, B, T> mulDiagMatrixDifferentSize(DiagMatrix<D, B, T> m1, DiagMatrix<D, B, T> m2) {
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
*/

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

template<int N, typename T>
using RawMatrix = std::array< std::array<T, N>, N>;


template<int N, typename T>
void printRaw(const RawMatrix<N, T> &raw) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            std::cout << std::setprecision(3) <<  std::left << std::setw(MAT_COLUMN_WIDTH) << raw[i][j] << " ";
        }
        std::cout << "\n";
    }
}


template<int N, typename T>
RawMatrix<N, T> inputRaw() {

    std::cout << "please input matrix:\n";
    RawMatrix<N, T> input;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            std::cin >> input[i][j];
        }
    }

    std::cout << "input matrix:\n";
    printRaw<N, T>(input);
    std::cout << "\n";

    return input;

}




template<int D, int B, typename T>
RawMatrix<D*B,  T> mkRawMatrix(DiagMatrix<D, B, T> m) {
    RawMatrix<D*B, T> out;
    for(int i = 0; i < D*B; i++) {
        for(int j = 0; j < D*B; j++) {
            const int iblock = i / D;
            const int jblock = j / D;
            const int iinner = i % D;
            const int jinner = j % D;

            if(iinner == jinner) {
                out[i][j] = m.blocks[iblock][jblock][iinner];
            }
            else {
                out[i][j] = 0;
            }
        }
    }
    return out;
}

template<int N, typename T>
RawMatrix<N, T> mulRawMatrix(RawMatrix<N, T> m1, RawMatrix<N, T>m2) {
    RawMatrix<N, T> raw;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            raw[i][j] = 0;
            for(int k = 0; k < N; k++) {
                raw[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    return raw;
}


// ouch, this cost is painful. Just switch to CML.
// TODO: switch to unique_ptr<, custom_dtor>;
template<int N, typename T>
MATRIX *mkCMLFromRaw(RawMatrix<N, T> r) {
    MATRIX *cml = cml_new(N, N);
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            // TODO: look into cml_set_{row, col}
            cml_set(cml, i, j, r[i][j]);
        }
    }
    return cml;
}

// perform matrix inversion using CML.
template<int N, typename T>
RawMatrix<N, T> invRawMatrixCML(RawMatrix<N, T> m, bool &success) {
    RawMatrix<N, T> out;
    MATRIX *cmlm = mkCMLFromRaw<N, T>(m);

    assert(cmlm != nullptr);
    success = cml_inverse(cmlm, NULL);
    if (!success) return out;

    // I am almost 100% sure I can memcpy() between these two, but just
    // to be safe, I hesitate to do that -- memory layouts and whatnot.
    // Let's get this running first, optimise later.
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            out[i][j] = cml_get(cmlm, i, j);
        }
    }

    return out;
}

template<int N, typename T>
RawMatrix<N, T> mkRawIdentity() {
    RawMatrix<N, T> id;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            id[i][j] = i == j;
        }
    }
    return id;

}



// Unfortunately,we are in C land, and we must mutate for performance.
// In light of this, we take a reference to the matrix and the
// rows to be switched
// mut = mutate
template<int N, typename T>
void swapRowsMutRawMat(RawMatrix<N, T> &m, int r1, int r2) {
    assert(r1 >= 0 && r1 <= N && "row out of bounds");
    assert(r2 >= 0 && r2 <= N && "row out of bounds");
    T temp;
    for(int c = 0; c < N; c++) {
        temp = m[r1][c];
        m[r1][c] = m[r2][c];
        m[r2][c] = temp;
    }
}

// AxmyRow = "Ax - y" a row (a play on "axpy") 
// Perform the row operation: Rtarget <- Rtarget - scale * Rsrc
template<int N, typename T>
void AxmyColumn(RawMatrix<N, T> &m, T scale, int csrc, int ctarget) {
    for(int r = 0; r < N; r++) {
        m[r][ctarget] -= scale * m[r][csrc];
    }
}


template<int N, typename T>
void AxmyRow(RawMatrix<N, T> &m, T scale, int rsrc, int rtarget) {
    for(int c = 0; c < N; c++) {
        m[rtarget][c] -= scale * m[rsrc][c];
    }
}



// Perform the column operation: ctarget <- ctarget * scale
template<int N, typename T>
void scaleColumn(RawMatrix<N, T> &m, T scale, int ctarget) {
    for(int r = 0; r < N; r++) {
        m[r][ctarget] *= scale;
    }
}


// Perform the row operation: rowtarget <- rowtarget * scale
template<int N, typename T>
void scaleRow(RawMatrix<N, T> &m, T scale, int rtarget) {
    for(int c = 0; c < N; c++) {
        m[rtarget][c] *= scale;
    }
}



// Perform matrix inversion using naive gauss jordan.
template<int N,  typename T>
RawMatrix<N, T> invRawMatrixOurs(RawMatrix<N, T> m, bool &success) {
    RawMatrix<N, T> out = mkRawIdentity<N, T>();

#ifdef DEBUG
    std::cout << "INPUT\n";
    printRaw<N, T>(m);
    std::cout <<"==\n";
#endif

    // With each row, kill every column other than the current diagonal column
    // with the 1st row, kill all coeffs other than that of the first column *in the first row*
    // with the 2nd row, kill all coeffs other than that of the 2nd column *in the second row*
    // We use "d" to denote that we are interested in the *d*iagonal element (pivot)
    // Src = doing the killing (the pivot we use to kill)
    // Column = going to be killed (the columns we make 0 at the source row)
    for(int dsrc = 0; dsrc < N; dsrc++) {

        // Scale our row so that we get a 1 at our pivot position
        // a   CUR   c   d
        // transforms to:
        // a/CUR 1     c/CUR  d /CUR
        
#ifdef DEBUG
        std::cout << "pivot: " << m[dsrc][dsrc] << "\n";
#endif

        // 1. SWAP PIVOTS IF PIVOT IS 0
        if (m[dsrc][dsrc] == 0){
            // We do need to pivot, so pivot using the largest row.
            // First find the largest row
            int biggest_row = dsrc;

            // Look for a row to pivot with
            for(int rpivot = dsrc+1; rpivot < N; rpivot++) {
                // In the rth block in the rpivotent(b)th collumn, look at the dth element.
                if (std::abs(m[rpivot][dsrc]) > std::abs(m[biggest_row][dsrc])) {
                    biggest_row = rpivot;
                }
            }

            // we now looked for the biggest row. If it is *equal* to the current
            // row, then we have all 0's on this dimension of the space,
            // so we're screwed, since this dimension is being sent into the null space.
            // Give up.
            if (biggest_row == dsrc) {
                success = false;
                return out;
            }
#ifdef DEBUG
            std::cout << "pivoting row(" << dsrc << ") with row(" << biggest_row <<")\n";
#endif

            // We have a row to pivot
            swapRowsMutRawMat<N, T>(m, biggest_row, dsrc);
            swapRowsMutRawMat<N, T>(out, biggest_row, dsrc);
        }


#ifdef DEBUG
        std::cout << "===\n";
        std::cout << __LINE__ << ":MATRIX AFTER PIVOT(" << dsrc <<"):\n";
        printRaw<N, T>(m);
        std::cout << __LINE__ << ":OUT AFTER PIVOT(" << dsrc <<"):\n";
        printRaw<N, T>(out);
        std::cout << "===\n";
#endif


        // 2. SCALE PIVOT TO BE 1
        {
            const T pivot = m[dsrc][dsrc];
            assert(pivot != 0 && "pivot is 0!");
            scaleRow<N, T>(m, (T)(1.0 / pivot), dsrc);
            scaleRow<N, T>(out, (T)(1.0 / pivot), dsrc);
        }


#ifdef DEBUG
        std::cout << "===\n";
        std::cout << __LINE__ << ":MATRIX AFTER SCALING(" << dsrc <<"):\n";
        printRaw<N, T>(m);
        std::cout << __LINE__ << ":OUT AFTER SCALING(" << dsrc <<"):\n";
        printRaw<N, T>(out);
        std::cout << "===\n";
#endif


        // 3. PERFORM ROW ELIMINATION
        // 1 a b
        // p q r
        // x y z
        // ->
        // 1 a b
        // 0 q' r'
        // 0 y' z'
        // 
        // ->
        // 1 a b
        // 0 1 r''
        // 0 y' z'
        // ->
        // 1 0 b'
        // 0 1 r''
        // 0 0 z'
        // ->
        for(int rtarget = 0; rtarget < N; rtarget++) {
            if (dsrc == rtarget) continue;

            // Pick the element in the target row, at the (pivot column = src row)
            const T scale = m[rtarget][dsrc];

            AxmyRow<N, T>(m, scale, dsrc, rtarget);
            AxmyRow<N, T>(out, scale, dsrc, rtarget);
        }

#ifdef DEBUG
        std::cout << "===\n";
        std::cout << __LINE__ << ":MATRIX AFTER ROW MANIPULATION(" << dsrc <<"):\n";
        printRaw<N, T>(m);
        std::cout << __LINE__ << ":OUT AFTER ROW MANIPULATION(" << dsrc <<"):\n";
        printRaw<N, T>(out);
        std::cout << "===\n";
#endif

    } // end dsrc loop


#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":FINAL MATRIX:\n";
    printRaw<N, T>(m);
    std::cout << __LINE__ << ":FINAL OUT:\n";
    printRaw<N, T>(out);
    std::cout << "===\n";
#endif

    success = true;
    return out;
}


enum class LogLevel {
    LogOn = 1,
    LogOff = 0
};
template<int N, typename T>
bool isRawEqual(RawMatrix<N, T> r1, std::string r1Name, 
        RawMatrix<N, T> r2, std::string r2Name, 
        const T eps, LogLevel l) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            if (std::abs(r1[i][j] - r2[i][j]) > eps) {

                // we need to log
                if (l == LogLevel::LogOn) {
                    std::cout << "matrices differ at: (" << i << "," << j <<")\n";
                    std::cout << r1Name << "[" << i << "][" << j << "] = " << r1[i][j] << "\n";
                    std::cout << r2Name << "[" << i << "][" << j << "] = " << r2[i][j] << "\n";

                    std::cout<<r1Name << ":\n";
                    printRaw<N, T>(r1);

                    std::cout<<"\n"<<r2Name << ":\n";
                    printRaw<N, T>(r2);
                }


                return false;
            }
        }
    }
    return true;
}

template<typename FloatT>
FloatT genRandFloat(const int mod, const int SIZE) { 
    const FloatT sign = rand() % 2 ? 1 : -1;
    const FloatT val = rand() % mod;

    return (sign * val) / (FloatT)SIZE;
}

// NOTE: actually use a PRNG, don't (rand() % mod) / SIZE, this will bias
// the results
template<int D, int B, typename FloatT>
DiagMatrix<D, B, FloatT> genRandDiagFloatMatrix(const int mod = 8, const int SIZE = 1) {
    DiagMatrix<D, B, FloatT> diag;
    for(int i = 0; i < B; i++) {
        for(int j = 0; j < B; j++) {
            for(int k = 0; k < D; k++) {
                diag.blocks[i][j][k] = genRandFloat<FloatT>(mod, SIZE);
            }
        }
    }

    return diag;
}


template<int N, typename FloatT>
RawMatrix<N, FloatT> genRandRawFloatMatrix(const int mod = 8, const int SIZE = 1) {
    RawMatrix<N, FloatT> raw;
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            raw[i][j] = genRandFloat<FloatT>(mod, SIZE);
        }
    }
    return raw;
}

// TODO: remove code duplication?
template<int D, int B, typename T>
void checkMatmulSameSize(DiagMatrix<D, B, T> d1, DiagMatrix<D, B, T> d2, const T eps) {
    //DiagMatrix<D, B, T> diag  = mulDiagMatrixSameSize(d1, d2);
    DiagMatrix<D, B, T> diag  = mulDiagMatrixGeneral(d1, d2);
    std::cout<<"\nMULDIAG:\n";
    printDiag<D, B, T>(diag);
    std::cout<<"\n====\n";

    RawMatrix<D*B, T> raw = mulRawMatrix<D*B, T>(mkRawMatrix<D, B, T>(d1), 
            mkRawMatrix<D, B, T>(d2));

    std::cout<<"\nMULRAW:\n";
    printRaw<D*B, T>(raw);

    RawMatrix<D*B, T> diag2raw = mkRawMatrix(diag);

    const bool isEqual = isRawEqual<D*B, T>(raw, "raw", diag2raw, "diag", eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");
}

// I have no idea if the inverse of these matrices continues to be a
// diagonal matrix, so I'm going to run experiments and find out :)
// Yay to knowing how to program.
// https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/matrix-inverse
// TODO: make this a Maybe
template<int D, int B, typename T>
DiagMatrix<D, B, T> invDiagMatrix(DiagMatrix<D, B, T> m, bool &success) {
    DiagMatrix<D, B, T> out;

    // for every D, we get BxB solutions to solve
    for(int d = 0; d < D; d++) {
        RawMatrix<B, T> subproblem;

        // gather subproblem
        for(int i = 0; i < B; i++) {
            for(int j = 0; j < B; j++) {
                subproblem[i][j] = m.blocks[i][j][d];
            }
        }

        RawMatrix<B, T> subsoln = invRawMatrixOurs<B, T>(subproblem, success);
        if (!success) { return out; }
        // scatter subsolution;
        for(int i = 0; i < B; i++) {
            for(int j = 0; j < B; j++) {
                out.blocks[i][j][d] = subsoln[i][j];
            }
        }
    }
    success = true;
    return out;

};


enum CheckInverseResult {
    CIRSuccess,
    CIRFail,
    CIRNonInvertible
};


// sanity check that the two matrices are inverses of each other
// by multiplying them and checking that the result is almost-identity.
template<int N, typename T>
bool checkInverseByMatmul(RawMatrix<N, T> m1, RawMatrix<N, T> m2, float eps) {
    RawMatrix<N, T> mul = mulRawMatrix<N, T>(m1, m2);

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            const T val = mul[i][j];
            if (i == j) {
                // id[i][j] = 1
                // assert (std::abs(1 - val) < eps);
                if (std::abs(1 - val) > eps) return false;
            }
            else {
                // id[i][j] = 1
                // assert(std::abs(val) < eps);
                if(std::abs(val) >= eps) return false;
            }
        }
    }

    return true;
}

template<int D, int B, typename T>
CheckInverseResult checkInverse(DiagMatrix<D, B, T> d, const T eps) {

    bool success = false;
    const RawMatrix<D*B, T> rawd = mkRawMatrix<D, B, T>(d);
    const RawMatrix<D*B, T> refinv = invRawMatrixCML<D*B, T>(rawd, success);
    if (!success) return CIRNonInvertible;

    // CML is screwed, it doesn't check if the matrix is ill conditioned, so 
    // we manually check if the supposed inverse is an inverse by performing
    // matmul.
    if (!checkInverseByMatmul<D*B, T>(mkRawMatrix<D, B, T>(d), refinv, eps)) {
        return CIRNonInvertible;
    }

    // we don't check for success here, so let's first check for success in the case of
    // raw
    bool diag_success = false;
    DiagMatrix<D, B, T> diag_inverse  = invDiagMatrix(d, diag_success);

    assert(diag_success == true && "invDiag unable to invert matrix that invRaw can!");


    const bool isEqual = isRawEqual<D*B, T>(refinv, "reference inverse (raw)", 
            mkRawMatrix<D, B, T>(diag_inverse), "test inverse (from diag)", 
            eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");

    if (isEqual) return CIRSuccess;
    return CIRFail;
}
