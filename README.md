# Matrix solutions to the "Human Analysis Lab" at Michigan State university.


## Question to be solved
** [Click here for link](http://hal.cse.msu.edu/misc/join/) **

Consider a matrix A of size `R^((nd) * nd)`, with blocks of diagonal matrices. 
Each diagonal matrix has `d` elements of the diagonal. The full matrix has `n * n`
of these diagonal blocks.

Construct efficient algorithms to perform matrix multiplication and inversion.
Provide proofs of complexity of algorithms.


## Why C++
- I like the language, thanks to nice type-level safety features to make sure
  I don't mess up with indexing, etc. 

- I can run TySan and ASAN, two very powerful sanitizers to make sure my code
  does not do stupid things. This is almost as good as having haskell-like checks
  with very little of the slowdowns.

- It lets me use Polly to see if we get interesting perf gains automatically.
  This isn't even cheating since I worked some amount on polly ;)

## Benchmarks (TODO)
