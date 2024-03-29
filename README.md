3 Matrix solutions to the "Human Analysis Lab" at Michigan State university.
**[Solution Report link here](report/main.pdf)**


## Question to be solved
**[Click here for link](http://hal.cse.msu.edu/misc/join/)**

Consider a matrix A of size `R^((nd) * nd)`, with blocks of diagonal matrices. 
Each diagonal matrix has `d` elements of the diagonal. The full matrix has `n * n`
of these diagonal blocks.

Construct efficient algorithms to perform matrix multiplication and inversion.
Provide proofs of complexity of algorithms.

## Thoughts as I solve this

- There is some annoying bug with block size ≠ diagonal size, will fix.
- Of course not, there's no bug, I'm just retarded. Assume `D = 7, B = 3`, all
of my assumptions about indexing (neatly picking up a block) will screw up?
Check that I'm not assuming something about the shape. I'm pretty sure I am.
The problem is most likely at the part where when you block multiple, you'll
take a block of size `DxD`, which needs to be picked up correctly from the
other matrices. Too sleepy right now.


- I'll probably do this for now: add a method called `ix(i, j)` which allows
for "real" indeces to be passed, from which we then figure out how to index
into the matrix. This is inefficient, so I assume you could provide a template
instance for `matmul<D, D, T>` which is specialized for the case when `D = B`.
This can be used to implement a "naive" inverse, but this is obviously bad.

- No no, the chunking works, I was just trashing memory like a bad C++ programmer.
Use the `GLIBCXX` debug check to figure out what was screwing up, and now
matmul works :) Next step, implement matrix inverse using gaussian elimination.

- Also note that currently, I'm only implementing this on matrices that have
the same block/diagonal size, I should generalize this to matrices with
different block/diagonal size, but with the same total size.

- This brings up a question, under what conditions are two such matrices
going to produce another diagonal matrix? Intuitively, I think when one
of their diagonal sizes is a multiple of the other. not sure what happens
when the diagonal sizes are coprime.

- Anyway, now I'm looking into inverses. Intuitively, the inverse of such a 
matrix should look similar, because inverses are horrible in general ;) 
Plus, these matrices maybe sparse, and inverses of sparse matrices
need not be sparse (I do not remember how I know this fact, but I googled
and [found the math.se answer again (link)](https://math.stackexchange.com/questions/471136/inverse-of-sparse-matrix-is-not-generally-sparse).

- Hm, this is nice and unexpected -- the inverse matrices are also in blocks!
cool, now let's try to understand why. Here is a sample matrix and its inverse:
```
MATRIX:
      5       0       0       0      -5       0       0       0      -0       0       0       0 
      0      -4       0       0       0      -6       0       0       0       0       0       0 
      0       0      -2       0       0       0      -0       0       0       0      -4       0 
      0       0       0      -5       0       0       0       6       0       0       0      -3 
      5       0       0       0      -6       0       0       0       0       0       0       0 
      0       4       0       0       0       7       0       0       0       7       0       0 
      0       0       7       0       0       0       1       0       0       0      -1       0 
      0       0       0      -3       0       0       0      -5       0       0       0       2 
      6       0       0       0       6       0       0       0      -7       0       0       0 
      0       6       0       0       0      -4       0       0       0      -3       0       0 
      0       0      -1       0       0       0       4       0       0       0      -5       0 
      0       0       0       4       0       0       0       1       0       0       0       7 
INVERSE:
1.2     0       0       0       -1      0       0       0       0       0       0       -0      
0       -0.0199 0       0       0       0.0511  0       0       0       0.119   0       -0      
0       0       0.00877 0       0       0       0.14    0       0       0       -0.0351 -0      
0       0       0       -0.12   0       0       0       -0.146  0       0       0       -0.00974 
1       0       0       0       -1      0       0       0       0       0       0       -0      
0       -0.153  0       0       0       -0.0341 0       0       0       -0.0795 0       -0      
0       0       -0.316  0       0       0       -0.0526 0       0       0       0.263   -0      
0       -0      0       0.0942  0       0       0       -0.0747 0       0       0       0.0617  
1.89    0       0       0       -1.71   0       0       0       -0.143  0       0       -0      
0       0.165   0       0       0       0.148   0       0       0       0.0114  0       -0      
0       0       -0.254  0       0       0       -0.0702 0       0       0       0.0175  -0      
0       0       0       0.0552  0       0       0       0.0942  0       0       0       0.14    
```

To make sure I wasn't going crazy, I wrote a little test that actually multiplies
the original and its supposed inverse, and it actually works out to the identity matrix.

- Hey, this makes sense! consider a system like this:

```
| x y | z w
+ ----+---
| 1 0 | 3 0
| 0 2 | 0 4
+ ----+----
| 2 0 | 9 0
| 0 4 | 0 3
```

Now, if we write down the linear equations that this matrix represents, what
we're getting is like,
```
x + 3z
2x + 9z
--
2y + 4w
4y + 3w
```

So, we can look at this as trying to invert two *smaller* set of *independent*
matrices:

```
| x z 
+ ----
| 1 3
| 2 9 
```

```
| y w
+ ----
| 2 4
| 4 3 
```

Or at least, that's the theory at this point.
I'm going to implement this as follows:
    - 1. Use the `cml_inverse` to find the inverse of the "chunks", so I reduce
          the problem by one level of difficulty: first I check that my hypothesis
          is correct (we can invert the independent system of equations).
    - 2. Now that our hypothesis is tested and works out, implement the gauss jordan
         pivot-then-row-reduce scheme that we are all familiar with!

Note that this explanation also sanity checks, because when `D = 1`, what we 
conclude is that we need to pick each equation from each row, which reduces
to "regular" matmul. nice!


- Yay, conjecture works (when I use the `cml_inverse` implementation). Now let's
implement gauss-jordan.

- Note that this suggests a complexity of `O(D * Inverse(B * B matrices)) = O(D * B^3)`,
which is lower than the naive `O((D * B)^3) = O(D^3 * B^3)`

- This is unrelated (as in, it is engineering), but I should change `RawMatrix<D, B, T>`
to `RawMatrix<N, T>`. It makes no sense for `RawMatrix` to have two degrees
of freedom (in fact, it's quite confusing).

- OK, I now have a test cradle for my matrix inversion - one called
`invRawMatrixCML` which will be the reference implementation, and one call
`invRawMatrixOurs` which will be the implementation I create using gauss-jordan.

- Some meta-engineering: change `int` to `size_t` everywhere, fix the shitty
printing in some places, create operator overloads for `*`, `<<` and create a
benchmarking rig using google's bench library.

- OK, finished with inverse. I thought I was wrong, when I actually wasn't --
It was the fact that the CML library screws up on non-invertible matrices.
So, now I check that the output of CML is actually correct by computing the matmul
of A * inv(A) ~= Identity

- Next step, generalizing matmul to different block and diagonal sizes. Since
our matrices are square, it is safe to assume that they are square matrices of
the form `(B1, D1)`, `(B2, D2)`, with the condition that
`B1 * D1 = B2 * D2` (since inner dimensions must be equal).
Intuition suggests that the resultant should be of the form `(B1 * D1 / LCM (D1, D2), LCM(D1, D2))`.
I'm going to add  a flag to take matrices and see what the resultants are.
Flag is called `mulexperimentdifferentsize`.

- Eyeballing, it seems to be actually `(GCD(B1, B2), D1 * B1 / GCD(B1, B2))`
  Will see what the actual math is.


- Getting to the math, here's a rough sketch (I'm not convinced by it).

```
Let X = (depth D, blocks B)
Let Y =  (depth D', blocks B')

Now, X(i, j) is non-zero whenever [i/B] % D == [j/B] %D
Similarly, Y(i, j) is non-zero when [i/B]' % D' == [j/B'] % D',
where [a] = floor(a).


Z = X * Y

For the matrices to be multipliable, D * B = D' * B'.

Z(i, j) = \sum_{k=1}^(D * B) X(i, k) * Y(k, j)

The non-zero entries in a matrix M(p, q) of type (B0, D0) will be in the
set NONZERO(B0, D0) =  {(αB + δ, βB + δ) | 0 <= α, β < B0, 0 <= δ <= D0)}

For X, Y to be non-zero we require that *BOTH*:
(i, k) ∈ NONZERO(B, D) AND 
(k, j) ∈ NONZERO(B', D')

That is, all points that are in 
{(αD + δ, βD + δ) | 0 <= α, β < B, 0 <= δ < D)} ∩ 
{(α'D' + δ', β'D' + δ') | 0 <= α', β' < B', 0 <= δ' < D')}.

I'll use <D> = {kB | k ∈ naturals } to borrow some ring theory notation
(<D> = principal ideal of B). I'll use N/D to refer to the set of naturals.
[0..D)

Written this way, we are looking for:

(<D> + D) ∩ (<B'> + D') = 
(<B> ∩ <B'>) + (<B> ∩ D') + (<B'> ∩ D) + (D ∩ D') = 
(<gcd(B, B')>=B0) + (?=D0)

We can identify what (?=D0) is since we now have B0. We know that
B * D = B0 * D0 ⇒ D0 = B * D / gcd(B, B').

However, this is very unsatisfying to me (the arrival at what D0 is),
so I want a cleaner proof of this.
```

- Wrote the report, cleaned up the general matmul proof to be more conceptual.
I need to add a figure, which I'll do now.

- I think I know where the slowdown comes from. Consider multiplying 
a diagonal matrix with a "full" matrix. In our algorithm, we will wind up
blocking the diagonal matrix with `1x1` blocks, out of which most blocks are dead.
What we should be able to do this this:
    1. Block at `D' = gcd(D1, D2)`
    2. Characterize dead blocks in `D1`, `D2` and completely ignore them
This should allow us to regain our asymptotics, but it's not fully clear to
me how to perform this "regain".



### Matmul
Matmul is not so hard, since we can perform matmul across blocks (matmul is
        fully parallel across all 3 loops, so we can permute the loops however we want,
        using blocks is one particular permutation). So we can simply multiply across
the blocks --- the blocks are diagonal, so we just need to multiply the diagonal
elemens.

This puts us at a time complexity of `O(B * B * B * D)` 
where `B` is the number of blocks
and `D` is the diagonal length. 

This is clearly better than the naive solution with is `O((B * D) ^3)`

### Inverse
I'm sure literature on this thing exists, but I'm not going to lookup literature
till I've solved it to my satisfaction. Plus I'm not sure if looking stuff up
is considered cheating.

Intuitively, some sort of LU-style decomposition should be pretty good, because
of the diagonal-ness of the system. I'll have to think about this.

Maybe it's possible to find the eigenbasis of this beast easily? That would
also make it nice to write the inverse. I believe not, because choosing 
`B = 1` gives us a "regular" matrix.


## Why C++
- I like the language, thanks to nice type-level safety features to make sure
I don't mess up with indexing, etc. 

- I can run TySan and ASAN, two very powerful sanitizers to make sure my code
does not do stupid things. This is almost as good as having haskell-like checks
with very little of the slowdowns.

- It lets me use Polly to see if we get interesting perf gains automatically.
This isn't even cheating since I worked some amount on polly ;)

## Benchmarks (TODO)

## Thanks to:
- [CML](https://github.com/MichaelJWelsh/cml), because I didn't trust my own
benchmark inverse implementation to be 100% correct.
- [cxxopts](https://github.com/jarro2783/cxxopts), a command line parser for C++.
