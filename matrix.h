#define _GLIBCXX_DEBUG
#include <iostream>
#include <iomanip>
#include <array>
#include <complex>
#include <assert.h>
#define CML_IMPLEMENTATION
#include "cml.h"

#define DEBUG 1

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
RawMatrix<D, B, T> inputRaw() {

    std::cout << "please input matrix:\n";
    RawMatrix<D, B, T> input;
    for(int i = 0; i < D*B; i++) {
        for(int j = 0; j < D*B; j++) {
            std::cin >> input[i][j];
        }
    }

    std::cout << "input matrix:\n";
    printRaw<D, B, T>(input);
    std::cout << "\n";

    return input;

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

// perform matrix inversion using CML.
template<int D, int B, typename T>
RawMatrix<D, B, T> invRawMatrixCML(RawMatrix<D, B, T> m, bool &success) {
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

template<int D, int B, typename T>
RawMatrix<D, B, T> mkRawIdentity() {
    RawMatrix<D, B, T> id;
    for(int i = 0; i < B * D; i++) {
        for(int j = 0; j < B * D; j++) {
            id[i][j] = i == j;
        }
    }
    return id;

}


// Unfortunately,we are in C land, and we must mutate for performance.
// In light of this, we take a reference to the matrix and the
// rows to be switched
// mut = mutate
template<int D, int B, typename T>
void swapRowsMutRawMat(RawMatrix<D, B, T> &m, int r1, int r2) {
    assert(r1 >= 0 && r1 <= B * D && "row out of bounds");
    assert(r2 >= 0 && r2 <= B * D && "row out of bounds");
    T temp;
    for(int c = 0; c < B * D; c++) {
        temp = m[r1][c];
        m[r1][c] = m[r2][c];
        m[r2][c] = temp;
    }
}

// AxmyRow = "Ax - y" a row (a play on "axpy") 
// Perform the row operation: Rtarget <- Rtarget - scale * Rsrc
template<int D, int B, typename T>
void AxmyColumn(RawMatrix<D, B, T> &m, T scale, int csrc, int ctarget) {
    for(int r = 0; r < B * D; r++) {
        m[r][ctarget] -= scale * m[r][csrc];
    }
}


template<int D, int B, typename T>
void AxmyRow(RawMatrix<D, B, T> &m, T scale, int rsrc, int rtarget) {
    for(int c = 0; c < B * D; c++) {
        m[rtarget][c] -= scale * m[rsrc][c];
    }
}



// Perform the column operation: ctarget <- ctarget * scale
template<int D, int B, typename T>
void scaleColumn(RawMatrix<D, B, T> &m, T scale, int ctarget) {
    for(int r = 0; r < B * D; r++) {
        m[r][ctarget] *= scale;
    }
}


// Perform the row operation: rowtarget <- rowtarget * scale
template<int D, int B, typename T>
void scaleRow(RawMatrix<D, B, T> &m, T scale, int rtarget) {
    for(int c = 0; c < B * D; c++) {
        m[rtarget][c] *= scale;
    }
}


            
// Perform matrix implementation using naive gauss jordan.
template<int D, int B, typename T>
RawMatrix<D, B, T> invRawMatrixOurs(RawMatrix<D, B, T> m, bool &success) {
    RawMatrix<D, B, T> out = mkRawIdentity<D, B, T>();

#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":input:\n";
    printRaw<D, B, T>(m);
    std::cout << "===\n";
#endif


    // (row, col) representation.
    // pivoting - look at blocks on the diagonal
    // We are pivoting *rows* around.
    for(int c = 0; c < D * B; c++) {
        if (m[c][c] != 0) continue;
        int biggest_row = c;

        // if we have a pivot element, continue

        // we don't have a pivot element on this column, so look for the
        // row that has the largest element on *this column*.
        // Not all rows of the matrix are useful, only rows of other
        // blocks that have the same "d" will be active.
        for(int curr = 0; curr < B * D; curr++) {
            // In the rth block in the current(b)th collumn, look at the dth element.
            if (std::abs(m[curr][c]) > std::abs(m[biggest_row][c])) {
                biggest_row = curr;
            }
        }

        // we now looked for the biggest row. If it is *equal* to the current
        // row, then we have all 0's on this dimension of the space,
        // so we're screwed, since this dimension is being sent into the null space.
        // Give up.
        if (biggest_row == c) {
            success = false;
            return out;
        }

        // We have a row to pivot
        swapRowsMutRawMat<D, B, T>(m, biggest_row, c);
        swapRowsMutRawMat<D, B, T>(out, biggest_row, c);

    }


#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":PIVOTED MATRIX:\n";
    printRaw<D, B, T>(m);
    std::cout << __LINE__ << ":PIVOTED OUT:\n";
    printRaw<D, B, T>(out);
    std::cout << "===\n";
#endif

    // Now we have a fully pivoted matrix, write an assert that checks this.
    for(int i = 0; i < B * D; i++) {
        assert(m[i][i] != 0 && "pivoted matrix has zeroes on diagonal!");
    }
    
    /*
    // scale everything down by the pivot so we get a 1 on the pivot
    // row
    for(int r = 0; r < B * D; r++) {
        const T scale = m[r][r];
        for(int c = 0; c < B * D; c++) {
            m[r][c] = m[r][c] / scale;
            out[r][c] = out[r][c] / scale;
        }
    }

#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":SCALED MATRIX:\n";
    printRaw<D, B, T>(m);
    std::cout << __LINE__ << ":SCALED OUT:\n";
    printRaw<D, B, T>(out);
    std::cout << "===\n";
#endif
    */


    // With each row, kill every column other than the current diagonal column
    // with the 1st row, kill all coeffs other than that of the first column *in the first row*
    // with the 2nd row, kill all coeffs other than that of the 2nd column *in the second row*
    // Src = doing the killing (the pivot we use to kill)
    // Column = going to be killed (the columns we make 0 at the source row)
    for(int rsrc = 0; rsrc < B * D; rsrc++) {

        // Scale our row so that we get a 1 at our pivot position
        // a   CUR   c   d
        // transforms to:
        // a/CUR 1     c/CUR  d /CUR
        {
            const T pivot = m[rsrc][rsrc];
            assert(pivot != 0 && "pivot is 0!");
            scaleRow<D, B, T>(m, (T)(1.0 / pivot), rsrc);
            scaleRow<D, B, T>(out, (T)(1.0 / pivot), rsrc);
        }

#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":MATRIX AFTER SCALE, BEFORE ROW MANIPULATION(" << rsrc <<"):\n";
    printRaw<D, B, T>(m);
    std::cout << __LINE__ << ":OUT AFTER SCALE, BEFORE ROW MANIPULATION(" << rsrc <<"):\n";
    printRaw<D, B, T>(out);
    std::cout << "===\n";

        static const float EPS = 1e-2;
        const T pivot = m[rsrc][rsrc];
        std::cout << "PIVOT FOR ROW(" << rsrc << ") = " << pivot << "\n";
        assert(std::abs(pivot - 1) < EPS && "pivot is not normalized!");
#endif

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
        for(int rtarget = 0; rtarget < B * D; rtarget++) {
            if (rsrc == rtarget) continue;

            // Pick the element in the target row, at the (pivot column = src row)
            const T scale = m[rtarget][rsrc];

            AxmyRow<D, B, T>(m, scale, rsrc, rtarget);
            AxmyRow<D, B, T>(out, scale, rsrc, rtarget);
        }

#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":MATRIX AFTER ROW MANIPULATION(" << rsrc <<"):\n";
    printRaw<D, B, T>(m);
    std::cout << __LINE__ << ":OUT AFTER ROW MANIPULATION(" << rsrc <<"):\n";
    printRaw<D, B, T>(out);
    std::cout << "===\n";
#endif

        //for(int c = 0; c < B * D; c++) {
        //    if (c == rsrc) {
        //        assert(std::abs(m[rsrc][c] - 1) < EPS);
        //    }
        //    else {
        //        assert(std::abs(m[rsrc][c]) < EPS);
        //    }
        //}


    } // end rsrc loop


#ifdef DEBUG
    std::cout << "===\n";
    std::cout << __LINE__ << ":FINAL MATRIX:\n";
    printRaw<D, B, T>(m);
    std::cout << __LINE__ << ":FINAL OUT:\n";
    printRaw<D, B, T>(out);
    std::cout << "===\n";
#endif

    success = true;
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


template<int D, int B, typename FloatT>
RawMatrix<D, B, FloatT> genRandRawFloatMatrix(const int mod = 8, const int SIZE = 1) {
    RawMatrix<D, B, FloatT> raw;
    for(int i = 0; i < D * B; i++) {
        for(int j = 0; j < D * B; j++) {
            raw[i][j] = genRandFloat<FloatT>(mod, SIZE);
        }
    }
    return raw;
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
DiagMatrix<D, B, T> invDiagMatrix(DiagMatrix<D, B, T> m, bool &success) {
    DiagMatrix<D, B, T> out;

    // for every D, we get BxB solutions to solve
    for(int d = 0; d < D; d++) {
        RawMatrix<B, 1, T> subproblem;

        // gather subproblem
        for(int i = 0; i < B; i++) {
            for(int j = 0; j < B; j++) {
                subproblem[i][j] = m.blocks[i][j][d];
            }
        }

        RawMatrix<B, 1, T> subsoln = invRawMatrixCML<B, 1, T>(subproblem, success);
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

template<int D, int B, typename T>
CheckInverseResult checkInverse(DiagMatrix<D, B, T> d, const T eps) {

    bool success = false;
    RawMatrix<D, B, T> raw = invRawMatrixCML<D, B, T>(mkRawMatrix<D, B, T>(d), success);
    if (!success) return CIRNonInvertible;

    // we don't check for success here, so let's first check for success in the case of
    // raw
    bool diag_success = false;
    DiagMatrix<D, B, T> diag_inverse  = invDiagMatrix(d, diag_success);

    assert(diag_success == true && "invDiag unable to invert matrix that invRaw can!");


    const bool isEqual = isRawEqual<D, B, T>(raw, "raw", mkRawMatrix<D, B, T>(diag_inverse), "diag", eps, LogLevel::LogOn);
    assert(isEqual && "matrices not equal!");

    if (isEqual) return CIRSuccess;
    return CIRFail;
}
