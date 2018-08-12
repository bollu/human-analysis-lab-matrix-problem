.PHONY=run

mat.out: matrix.cpp main.cpp matrix.h
	clang matrix.cpp main.cpp -o mat.out

run: mat.out
	./mat.out
