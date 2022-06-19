#pragma once

#include <math.h>
#include <algorithm>
#include <iostream>


const double PI = acos(-1);


class Vect3 {
	double inf = 1e18, eps = 0.00001;

public:
	double x, y, z, w;

	Vect3(double x = 0, double y = 0, double z = 0, double w = 1) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	Vect3(std::vector < double > init) {
		for (int i = 0; i < std::min(4, (int)init.size()); i++)
			(*this)[i] = init[i];
	}

	double& operator[](int index) {
		if (index == 0)
			return x;
		if (index == 1)
			return y;
		if (index == 2)
			return z;
		if (index == 3)
			return w;
	}

	bool operator ==(Vect3 other) {
		return x == other.x && y == other.y && z == other.z;
	}

	bool operator !=(Vect3 other) {
		return x != other.x || y != other.y || z != other.z;
	}

	void operator +=(Vect3 other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void operator -=(Vect3 other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}

	void operator *=(double other) {
		x *= other;
		y *= other;
		z *= other;
	}

	void operator /=(double other) {
		if (other != 0) {
			x /= other;
			y /= other;
			z /= other;
		}
		else {
			std::cout << "ERROR::VECT3::DIVISION\n" << "Division by zero.\n";
		}
	}

	Vect3 operator -() {
		return Vect3(-x, -y, -z);
	}

	Vect3 operator +(Vect3 other) {
		return Vect3(x + other.x, y + other.y, z + other.z);
	}

	Vect3 operator -(Vect3 other) {
		return Vect3(x - other.x, y - other.y, z - other.z);
	}

	Vect3 operator *(double other) {
		return Vect3(x * other, y * other, z * other);
	}

	double operator *(Vect3 other) {
		return x * other.x + y * other.y + z * other.z;
	}

	Vect3 operator ^(Vect3 other) {
		return Vect3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
	}

	Vect3 operator ^(double other) {
		return Vect3(pow(abs(x), other), pow(abs(y), other), pow(abs(z), other));
	}

	Vect3 operator /(double other) {
		if (other != 0)
			return Vect3(x / other, y / other, z / other);
		std::cout << "ERROR::VECT3::DIVISION\n" << "Division by zero.\n";
	}

	double length_sqr() {
		return *this * *this;
	}

	double length() {
		return sqrt(*this * *this);
	}

	Vect3 normalize() {
		return *this / (*this).length();
	}

	double cos_angle(Vect3 V) {
		return (*this * V) / ((*this).length() * V.length());
	}

	double sin_angle(Vect3 V) {
		return ((*this ^ V).length()) / ((*this).length() * V.length());
	}

	Vect3 reflect_vect(Vect3 N) {
		return N * (2 * (N * *this)) - *this;
	}

	Vect3 set_max(Vect3 V) {
		x = std::max(x, V.x);
		y = std::max(y, V.y);
		z = std::max(z, V.z);
		return *this;
	}

	Vect3 set_min(Vect3 V) {
		x = std::min(x, V.x);
		y = std::min(y, V.y);
		z = std::min(z, V.z);
		return *this;
	}

	bool in_angle(Vect3 a, Vect3 b) {
		return abs((a ^ *this).cos_angle(b ^ *this) + 1) < eps;
	}

	float* value_ptr() {
		return new float[3]{ (float)x, (float)y, (float)z };
	}

	std::vector < double > value_vector() {
		return { x, y, z };
	}

	void print() {
		std::cout << '(' << x << ", " << y << ", " << z << ")\n";
	}
};

Vect3 operator *(double x, Vect3 v) {
	return Vect3(v.x * x, v.y * x, v.z * x);
}
