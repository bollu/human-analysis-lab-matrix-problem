.PHONY=run

mat.out: main.cpp matrix.h
	clang main.cpp -o mat.out

run: mat.out
	./mat.out
