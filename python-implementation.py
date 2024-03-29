#!/usr/bin/env python3
import random
from fractions import gcd
from tqdm import tqdm

NUMTESTSPERSIZE = 10
MAXSIZE = 10
ALIGNMENTSTR = "%6s"
EPS = 1e-3

def matrawdim(m):
    return (len(m), len(m[0]))

# Do this in python becasuse there's no pesky type safety to get in the way :)
def matmulRaw(m1, m2):
    I, J = matrawdim(m1)
    I2, J2 = matrawdim(m2)

    assert J == I2 

    out = [[0 for _ in range(J2)] for _ in range(I)]

    for i in range(I):
        for j in range(J2):
            for k in range(J):
                # print("out[%s][%s](%s) += m1[%s][%s](%s) * m2[%s][%s](%s)" % (i, j, out[i][j], i, k, m1[i][k], k, j, m2[k][j]))
                out[i][j] += m1[i][k] * m2[k][j]
    return out

# (S, D)
def matdiagdim(m):
    assert len(m) == len(m[0])
    return (len(m), len(m[0][0]))


def strmatdiag(m):
    S, D = matdiagdim(m)

    out = ""
    for i in range(S * D):
        for j in range(S * D):
            si = i // D
            sj = j // D

            di = i % D
            dj = j % D

            if (di == dj):
                out += ALIGNMENTSTR % str(m[si][sj][di])
            else:
                out += ALIGNMENTSTR % "0"
        out += "\n";

    # out += ("-" * S * D) + "\n"

    return out

def printmatdiag(m):
    print(strmatdiag(m))

def printmatraw(m):
    out = ""
    for row in m:
        for i in row:
            out += ALIGNMENTSTR % str(i) 

        out += "\n"


    print(out)

def matdiagtoraw(m):
    S, D = matdiagdim(m)

    out = [[0 for _ in range(S * D)] for _ in range(S * D)]

    for i in range(S * D):
        for j in range(S * D):
            si = i // D
            sj = j // D

            di = i % D
            dj = j % D

            if (di == dj):
                out[i][j] = m[si][sj][di]
            else:
                out[i][j] = 0

    return out

def randmatdiag(S, D):
    MAX = 10
    return [[[random.randint(-MAX, MAX) for _ in range(D)] for _ in range (S)] for _ in range (S)]


def randmatraw(N):
    MAX = 10
    return [[[random.randint(-MAX,  MAX) for _ in range(N)] for _ in range (N)]]

def matmulDiagEqChunking(m1, m2):
    (S1, D1) = matdiagdim(m1)
    (S2, D2) = matdiagdim(m2)

    assert S1 == S2 and D1 == D2
    DNEW = D1
    SNEW = S1

    out = [[[0 for _ in range(DNEW)] for _ in range (SNEW)] for _ in range (SNEW)]

    for i in range(SNEW):
        for j in range(SNEW):
            for k in range (SNEW):
                for d in range(DNEW):
                    # print("> out[%s][%s][%s](%s) += m1[%s][%s][%s](%s) * m2[%s][%s][%s](%s)" % (i, j, d, out[i][j][d], i, k, d, m1[i][k][d], k, j, d, m2[k][j][d]))
                    out[i][j][d] += m1[i][k][d] * m2[k][j][d]

    return out

def newSDCoordToOld(SNEW, DNEW, SOLD, DOLD, isnew, jsnew, dnew):
    """ return an (isold, jsold, dold) pair in the old coordinate system from 
    a point in the new coordinate system (isnew, jsnew, dnew)

    (SNEW, DNEW) are the new sizes
    (SOLD, DOLD) are the old sizes

    returns:
        (isold, jsold, dold) if it lies on the diagonal of the
        old system. **None otherwise**
    """
    # shape preservation
    assert (SNEW * DNEW == SOLD * DOLD)
    # new is smaller than old
    assert (DNEW <= DOLD)
    # new neatly partitions old
    assert (DOLD % DNEW == 0)

    i = isnew * DNEW + dnew
    j = jsnew * DNEW + dnew

    isold = i // DOLD
    jsold = j // DOLD

    idold = i % DOLD
    jdold = j % DOLD

    # if idold == jdold, then it's on the diagonal of the old system,
    # otherwise it isn't, and we should consider it as 0
    # eg: SNEW = 6, DNEW = 1, SOLD = 2, DOLD = 3
    # inew = 0, jnew = 1, dnew = 0
    
    if idold == jdold:
        return (isold, jsold, idold)
    else:
        return None



# (m1, m2: [S][S][d])
def matmulDiagNonEqChunking(m1, m2):
    (S1, D1) = matdiagdim(m1)
    (S2, D2) = matdiagdim(m2)

    assert(S1 * D1 == S2 * D2)

    DNEW = gcd(D1, D2)
    SNEW = S1 * D1 // DNEW

    out = [[[0 for _ in range(DNEW)] for _ in range (SNEW)] for _ in range (SNEW)]

    for i in range(SNEW):
        for j in range(SNEW):
            for k in range (SNEW):
                for d in range(DNEW):
                    maybeOld = newSDCoordToOld(SNEW, DNEW, S1, D1, i, k, d)

                    m1val = None
                    if maybeOld is not None:
                        (iold, kold, dold) = maybeOld
                        m1val = m1[iold][kold][dold]
                    else:
                        m1val = 0
                    assert (m1val is not None)

                    m2val = None
                    maybeOld = newSDCoordToOld(SNEW, DNEW, S2, D2, k, j, d)
                    if maybeOld is not None:
                        (kold, jold, dold) = maybeOld
                        m2val = m2[kold][jold][dold]
                    else:
                        m2val = 0
                    assert (m2val is not None)



                    out[i][j][d] += m1val * m2val

    return out


def rawmatsub(m1, m2):
    assert matrawdim(m1) == matrawdim(m2)
    out = [[0 for _ in range(len(m1[0]))] for _ in range(len(m1))]

    for i in range(len(m1)):
        for j in range(len(m1[0])):
            out[i][j] = m1[i][j] - m2[i][j]

    return out

def rawmatnormsq(m):
    out = 0
    for r in m:
        for val in r:
            out += val * val
    return out

def rawmatloss(m1, m2):
    return rawmatnormsq(rawmatsub(m1, m2))


def test_general_matmul_for_sizes(s1, d1, s2, d2):
    assert s1 * d1 == s2 * d2

    d1 = randmatdiag(s1, d1)
    d2 = randmatdiag(s2, d2)

    r1 = matdiagtoraw(d1)
    r2 = matdiagtoraw(d2)

    #print ("D1:")
    #printmatdiag(d1)
    #print ("-" * 10)
    #print(d1)
    #print ("-" * 10)
    #print("R1:")
    #print ("-" * 10)
    #print(r1)
    #printmatraw(r1)


    #print ("=" * 20)
    #print ("D2:")
    #printmatdiag(d2)
    #print ("R2:")
    #printmatraw(r2)

    #dmul = matmulDiagEqChunking(d1, d2)
    dmul = matmulDiagNonEqChunking(d1, d2)
    rmul = matmulRaw(r1, r2)

    #print ("=" * 20)
    #print ("DMUL:")
    #printmatdiag(dmul)
    #print ("RMUL:")
    #printmatraw(rmul)

    LOSS =  rawmatloss(rmul, matdiagtoraw(dmul))
    #print ("=" * 20)
    #print ("loss: %s" % LOSS)
    assert (LOSS < EPS)


if __name__ == "__main__":
    sizes = [(s1, d1, s2, d2) for s1 in range(1, MAXSIZE) 
             for d1 in range (1, MAXSIZE) 
             for s2 in range(1, MAXSIZE) 
             for d2 in range(1, MAXSIZE) if s1 * d1 == s2 * d2]


    for (s1, d1, s2, d2) in tqdm(sizes):
        for _ in tqdm(range(NUMTESTSPERSIZE)):
            test_general_matmul_for_sizes(s1, d1, s2, d2)

