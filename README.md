3 Matrix solutions to the "Human Analysis Lab" at Michigan State university.


## Question to be solved
** [Click here for link](http://hal.cse.msu.edu/misc/join/) **

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
