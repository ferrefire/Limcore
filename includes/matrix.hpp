#pragma once

#include "point.hpp"

#include <iostream>
#include <stdint.h>
#include <array>

#define MATRIX_TEMPLATE template <uint32_t R, uint32_t C, ValidMatrixType T> \
	requires (ValidMatrixRange<R> && ValidMatrixRange<C>)

#define MATRIX_CAST_TEMPLATE template <ValidMatrixType CT>

template <uint32_t S>
concept ValidMatrixRange = (S >= 1 && S <= 4);

template <typename T>
concept ValidMatrixType = (std::is_floating_point<T>().value);

template <uint32_t R, uint32_t C, ValidMatrixType T = float>
requires (ValidMatrixRange<R> && ValidMatrixRange<C>)
class Matrix
{
	static const uint32_t size = R * C;

	private:
		std::array<T, R * C> data{};

	public:
		Matrix();
		Matrix(const std::array<T, R * C>& init);
		MATRIX_CAST_TEMPLATE
		Matrix(const Matrix<R, C, CT>& other);
		Matrix<R, C, T>& operator=(const Matrix<R, C, T>& other);
		MATRIX_CAST_TEMPLATE
		Matrix<R, C, T>& operator=(const Matrix<R, C, CT>& other);
		
		~Matrix();

		T& operator[](const uint32_t i);
		const T& operator[](const uint32_t i) const;
		T& operator()(const uint32_t row, const uint32_t col);
		const T& operator()(const uint32_t row, const uint32_t col) const;

		Matrix<R, C, T> operator+(const Matrix<R, C, T>& other) const;
		Matrix<R, C, T> operator-(const Matrix<R, C, T>& other) const;
		Matrix<R, C, T> operator*(const Matrix<R, C, T>& other) const;
		Point<T, C> operator*(const Point<T, C>& point) const;

		void operator+=(const Matrix<R, C, T>& other);
		void operator-=(const Matrix<R, C, T>& other);
		void operator*=(const Matrix<R, C, T>& other);

		void Scale(const Point<T, C>& scalar);
		void Translate(const Point<T, C>& translation);
		template <uint32_t CS = C> requires (CS == 4)
		void Rotate(const T& degrees, const Axis& axis);

		static Matrix<R, C, T> Identity();
		static Matrix<R, C, T> Scalar(const Point<T, C>& scalar);
		static Matrix<R, C, T> Translation(const Point<T, C>& translation);
		static Matrix<R, C, T> Rotation(const T& degrees, const Axis& axis);
};

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Matrix<R, C, T>& matrix);

typedef Matrix<3, 3, float> mat3;
typedef Matrix<4, 4, float> mat4;

typedef Matrix<3, 3, double> dmat3;
typedef Matrix<4, 4, double> dmat4;

#include "matrix.tpp"