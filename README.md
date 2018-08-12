# Matrix solutions to the "Human Analysis Lab" at Michigan State university.


## Question to be solved
** [Click here for link](http://hal.cse.msu.edu/misc/join/) **

Consider a matrix A of size `R^((nd) * nd)`, with blocks of diagonal matrices. 
Each diagonal matrix has `d` elements of the diagonal. The full matrix has `n * n`
of these diagonal blocks.

Construct efficient algorithms to perform matrix multiplication and inversion.
Provide proofs of complexity of algorithms.

## Thoughts as I solve this

### Matmul
Matmul is not so hard, since we can perform matmul across blocks (matmul is
fully parallel across all 3 loops, so we can permute the loops however we want,
using blocks is one particular permutation). So we can simply multiply across
the blocks --- the blocks are diagonal, so we just need to multiply the diagonal
elemens.

This puts us at a time complexity of `O(B * B * D)` 
where `B, B` are the block sizes, 
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
