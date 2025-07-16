#pragma once

#include <iostream>
#include <stdint.h>
#include <array>

#define MATRIX_TEMPLATE template <uint32_t R, uint32_t C> \
	requires (ValidRange<R> && ValidRange<C>)

template <uint32_t S>
concept ValidRange = (S >= 1 && S <= 4);

MATRIX_TEMPLATE
class Matrix
{
	const uint32_t size = R * C; 

	private:
		std::array<float, R * C> data{};

	public:
		Matrix();
		~Matrix();

		float& operator[](const uint32_t i);
		float& operator()(const uint32_t row, const uint32_t col);
		void operator+=(Matrix<R, C> other);
		void operator-=(Matrix<R, C> other);
		void operator*=(Matrix<R, C> other);
};

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, Matrix<R, C> matrix);

#include "matrix.tpp"