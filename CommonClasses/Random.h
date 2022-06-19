#pragma once

#include <random>
#include "Vect3.h"


class Random {
	std::mt19937_64 gen;

public:
	Random(int sd) {
		gen.seed(sd);
	}

	void seed(int sd) {
		gen.seed(sd);
	}

	double rand() {
		return ((double)gen()) / ((double)gen.max());
	}

	long long randint(long long l, long long r) {
		return ((long long)(gen() % (r - l + 1))) + l;
	}

	double randfloat(double l, double r) {
		return (r - l) * rand() + l;
	}

	Vect3 randvect3(Vect3 l, Vect3 r) {
		return Vect3(randfloat(l.x, r.x), randfloat(l.y, r.y), randfloat(l.z, r.z));
	}
};
