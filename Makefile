.PHONY=run
# I build LLVM from source for Polly, I need to pull in ASAN or change $PATH.
# I do this tomorrow.
# SANITIZERS="-fsanitize=address"
SANITIZERS=""

mat.out: main.cpp matrix.h cml.h
	clang main.cpp  -o mat.out -lstdc++ -std=c++17 -Wall  $(SANITIZERS) -lblas -llapack -g 

test: mat.out
	./mat.out --multest --invtest
run: mat.out
	./mat.out $?
