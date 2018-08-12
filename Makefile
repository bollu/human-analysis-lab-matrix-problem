.PHONY=run

mat.out: main.cpp matrix.h
	clang main.cpp -o mat.out -lstdc++ -std=c++14 -Wall -Werror

run: mat.out
	./mat.out
