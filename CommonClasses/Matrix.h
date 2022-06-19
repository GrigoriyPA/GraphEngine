#pragma once

#include <math.h>
#include <iostream>
#include <vector>
#include "Vect3.h"


std::vector < double > gauss(int n, int m, std::vector < std::vector < double > > mx, std::vector < double > ans, double eps = 0.000001) {
	for (int j = 0, i0 = 0; j < m; j++) {
		int k = -1;
		for (int i = i0; i < n; i++) {
			if (abs(mx[i][j]) > eps) {
				k = i;
				break;
			}
		}
		if (k == -1) {
			for (int i = 0; i < i0; i++)
				mx[i][j] = 0;
		}
		else {
			std::swap(mx[i0], mx[k]);
			std::swap(ans[i0], ans[k]);
			ans[i0] /= mx[i0][j];
			for (int i = m - 1; i >= j; i--)
				mx[i0][i] /= mx[i0][j];
			for (int i = 0; i < i0; i++) {
				ans[i] -= ans[i0] * mx[i][j];
				for (int ind = m - 1; ind >= j; ind--)
					mx[i][ind] -= mx[i0][ind] * mx[i][j];
			}
			for (int i = i0 + 1; i < n; i++) {
				ans[i] -= ans[i0] * mx[i][j];
				for (int ind = m - 1; ind >= j; ind--)
					mx[i][ind] -= mx[i0][ind] * mx[i][j];
			}
			i0++;
		}
	}
	std::vector < double > res(m, 0);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			if (abs(mx[i][j]) > eps)
				res[j] = ans[i];
		}
	}
	return res;
}


class Matrix {
	int s, c;
	std::vector < std::vector < double > > mx;

public:
	Matrix(std::vector < std::vector < double > > init = { {} }) {
		s = init.size();
		c = 0;
		if (s > 0)
			c = init[0].size();
		mx = init;
	}

	Matrix(Vect3 vx, Vect3 vy, Vect3 vz) {
		s = 4;
		c = 4;
		mx = {
			{ vx.x, vy.x, vz.x, 0 },
			{ vx.y, vy.y, vz.y, 0 },
			{ vx.z, vy.z, vz.z, 0 },
			{    0,    0,    0, 1 },
		};
	}

	Matrix(int s, int c, int vl) {
		this->s = s;
		this->c = c;
		mx.resize(s, std::vector < double >(c, vl));
	}

	std::vector < double >& operator[](int index) {
		return mx[index];
	}

	Matrix operator *(Matrix other) {
		if (c != other.size_s()) {
			std::cout << "ERROR::MATRIX::MULTIPLICATION\n" << "Incorrect matrix sizes.\n";
			return Matrix();
		}

		other.transp();
		std::vector < std::vector < double > > res(s, std::vector < double >(other.size_c(), 0));
		for (int i = 0; i < s; i++) {
			for (int j = 0; j < other.size_c(); j++) {
				for (int k = 0; k < c; k++)
					res[i][j] += mx[i][k] * other[j][k];
			}
		}

		return Matrix(res);
	}

	Vect3 operator *(Vect3 other) {
		if (s != 4 || c != 4) {
			std::cout << "ERROR::MATRIX::MULTIPLICATION\n" << "Incorrect matrix sizes.\n";
			return Vect3();
		}

		Vect3 res;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 4; j++)
				res[i] += mx[i][j] * other[j];
		}
		
		return res;
	}

	Matrix transp() {
		for (int i = 0; i < s; i++) {
			for (int j = i + 1; j < c; j++)
				std::swap(mx[i][j], mx[j][i]);
		}
		return *this;
	}

	Matrix inverse() {
		if (s != c) {
			std::cout << "ERROR::MATRIX::MULTIPLICATION\n" << "Incorrect matrix sizes.\n";
			return Matrix();
		}

		std::vector < double > ans(s * c, 0);
		std::vector < std::vector < double > > mx_k(s * c, std::vector < double >(s * c, 0));
		for (int i = 0; i < s; i++) {
			ans[i * c + i] = 1;
			for (int j = 0; j < c; j++) {
				for (int k = 0; k < s; k++)
					mx_k[i * c + j][j * s + k] = mx[i][k];
			}
		}

		std::vector < double > tmp = gauss(s * c, s * c, mx_k, ans);
		std::vector < std::vector < double > > res(s, std::vector < double >(c));
		for (int i = 0; i < s; i++) {
			for (int j = 0; j < c; j++)
				res[i][j] = tmp[j * s + i];
		}

		return Matrix(res);
	}

	int size_s() {
		return s;
	}

	int size_c() {
		return c;
	}

	float* value_ptr() {
		float* res = new float[s * c];
		for (int j = 0; j < c; j++) {
			for (int i = 0; i < s; i++)
				res[j * s + i] = mx[i][j];
		}

		return res;
	}

	void print() {
		for (int i = 0; i < s; i++) {
			for (int j = 0; j < c; j++)
				std::cout << mx[i][j] << " ";
			std::cout << "\n";
		}
	}
};

Matrix one_matrix(int n) {
	std::vector < std::vector < double > > res(n, std::vector < double >(n, 0));
	for (int i = 0; i < n; i++)
		res[i][i] = 1;

	return Matrix(res);
}

Matrix scale_matrix(Vect3 s) {
	return Matrix({
		{ s.x,   0,   0, 0 },
		{   0, s.y,   0, 0 },
		{   0,   0, s.z, 0 },
		{   0,   0,   0, 1 },
	});
}

Matrix scale_matrix(double s) {
	return Matrix({
		{ s, 0, 0, 0 },
		{ 0, s, 0, 0 },
		{ 0, 0, s, 0 },
		{ 0, 0, 0, 1 },
	});
}

Matrix trans_matrix(Vect3 trans) {
	return Matrix({
		{ 1, 0, 0, trans.x },
		{ 0, 1, 0, trans.y },
		{ 0, 0, 1, trans.z },
		{ 0, 0, 0, 1 },
	});
}

Matrix rotate_matrix(Vect3 axis, double angle) {
	axis = axis.normalize();
	double c = cos(angle), s = sin(angle), x = axis.x, y = axis.y, z = axis.z;

	return Matrix({
		{     c + x * x * (1 - c), x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0 },
		{ y * x * (1 - c) + z * s,     c + y * y * (1 - c), y * z * (1 - c) - x * s, 0 },
		{ z * x * (1 - c) - y * s, z * y * (1 - c) + x * s,     c + z * z * (1 - c), 0 },
		{                       0,                       0,                       0, 1 },
	});
}
