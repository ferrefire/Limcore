#pragma once

#include <iostream>
#include <stdint.h>
#include <array>

#define MATRIX_TEMPLATE template <uint32_t R, uint32_t C> \
	requires (ValidMatrixRange<R> && ValidMatrixRange<C>)

template <uint32_t S>
concept ValidMatrixRange = (S >= 1 && S <= 4);

MATRIX_TEMPLATE
class Matrix
{
	const uint32_t size = R * C; 

	private:
		std::array<float, R * C> data{};

	public:
		Matrix();
		Matrix(std::array<float, R * C> init);
		Matrix<R, C>& operator=(const Matrix<R, C>& other);
		~Matrix();

		float& operator[](const uint32_t i);
		const float& operator[](const uint32_t i) const;
		float& operator()(const uint32_t row, const uint32_t col);
		const float& operator()(const uint32_t row, const uint32_t col) const;

		Matrix<R, C> operator+(const Matrix<R, C>& other) const;
		Matrix<R, C> operator-(const Matrix<R, C>& other) const;
		Matrix<R, C> operator*(const Matrix<R, C>& other) const;
		void operator+=(const Matrix<R, C>& other);
		void operator-=(const Matrix<R, C>& other);
		void operator*=(const Matrix<R, C>& other);
};

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Matrix<R, C>& matrix);

#include "matrix.tpp"