#include "matrix.hpp"

#include <stdexcept>
#include <cmath>

MATRIX_TEMPLATE
Matrix<R, C, T>::Matrix()
{

}

MATRIX_TEMPLATE
Matrix<R, C, T>::Matrix(const std::array<T, R * C>& init)
{
	for (int i = 0; i < size; i++)
	{
		int r = i / R;
		int c = i % R;

		(*this)(r, c) = init[i];
	}
}



MATRIX_TEMPLATE
MATRIX_CAST_TEMPLATE
Matrix<R, C, T>::Matrix(const Matrix<R, C, CT>& other)
{
	for (int i = 0; i < size; i++) { this->data[i] = other[i]; }
}

MATRIX_TEMPLATE
Matrix<R, C, T>& Matrix<R, C, T>::operator=(const Matrix<R, C, T>& other)
{
	for (int i = 0; i < size; i++) { this->data[i] = other[i]; }

	return (*this);
}

MATRIX_TEMPLATE
MATRIX_CAST_TEMPLATE
Matrix<R, C, T>& Matrix<R, C, T>::operator=(const Matrix<R, C, CT>& other)
{
	for (int i = 0; i < size; i++) { this->data[i] = other[i]; }

	return (*this);
}

MATRIX_TEMPLATE
Matrix<R, C, T>::~Matrix()
{
	
}

MATRIX_TEMPLATE
T& Matrix<R, C, T>::operator[](const uint32_t i)
{
	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
const T& Matrix<R, C, T>::operator[](const uint32_t i) const
{
	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
const T& Matrix<R, C, T>::operator()(const uint32_t row, const uint32_t col) const
{
	uint32_t i = row + (col * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
T& Matrix<R, C, T>::operator()(const uint32_t row, const uint32_t col)
{
	uint32_t i = row + (col * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::operator+(const Matrix<R, C, T>& other) const
{
	Matrix<R, C, T> result;

	for (int i = 0; i < size; i++)
	{
		result[i] = data[i] + other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::operator-(const Matrix<R, C, T>& other) const
{
	Matrix<R, C, T> result;

	for (int i = 0; i < size; i++)
	{
		result[i] = data[i] - other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::operator*(const Matrix<R, C, T>& other) const
{
	Matrix<R, C, T> result;

	for (int r = 0; r < R; r++)
	{
		for (int c = 0; c < C; c++)
		{
			for (int i = 0; i < C; i++)
			{
				result(r, c) += (*this)(r, i) * other(i, c);
			}
		}
	}

	return (result);
}

MATRIX_TEMPLATE
Point<T, C> Matrix<R, C, T>::operator*(const Point<T, C>& point) const
{
	Point<T, C> result;

	for (int r = 0; r < R; r++)
	{
		for (int c = 0; c < C; c++)
		{
			result[r] += (*this)(r, c) * point[c];
		}
	}

	return (result);
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::operator+=(const Matrix<R, C, T>& other)
{
	for (int i = 0; i < size; i++)
	{
		data[i] += other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::operator-=(const Matrix<R, C, T>& other)
{
	for (int i = 0; i < size; i++)
	{
		data[i] -= other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::operator*=(const Matrix<R, C, T>& other)
{
	*this = *this * other;
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::Scale(const Point<T, C>& scalar)
{
	(*this) = Matrix<R, C, T>::Scalar(scalar) * (*this);
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::Translate(const Point<T, C>& translation)
{
	(*this) = Matrix<R, C, T>::Translation(translation) * (*this);
}

MATRIX_TEMPLATE
template <uint32_t CS> requires (CS == 4)
void Matrix<R, C, T>::Rotate(const T& degrees, const Axis& axis)
{
	(*this) = Matrix<R, C, T>::Rotation(degrees, axis) * (*this);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Identity()
{
	Matrix<R, C, T> result;

	for (int r = 0; r < R; r++)
	{
		for (int c = 0; c < C; c++)
		{
			result(r, c) = (r == c ? 1 : 0);
		}
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Scalar(const Point<T, C>& scalar)
{
	Matrix<R, C, T> result = Identity();

	for (int i = 0; i < C; i++)
	{
		result(i, i) *= scalar[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Translation(const Point<T, C>& translation)
{
	Matrix<R, C, T> result = Identity();
	
	for (int i = 0; i < C; i++)
	{
		result(i, C - 1) += translation[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Rotation(const T& degrees, const Axis& axis)
{
	Matrix<R, C, T> result = Identity();
	
	T radians = degrees * 0.0174532925;
	T cosTheta = cos(radians);
	T sinTheta = sin(radians);

	if (axis == Axis::x || axis == Axis::z) result(1, 1) = cosTheta;
	if (axis == Axis::x || axis == Axis::y) result(2, 2) = cosTheta;
	if (axis == Axis::y || axis == Axis::z) result(0, 0) = cosTheta;

	if (axis == Axis::x)
	{
		result(1, 2) = -sinTheta;
		result(2, 1) = sinTheta;
	}
	else if (axis == Axis::y)
	{
		result(0, 2) = -sinTheta;
		result(2, 0) = sinTheta;
	}
	else if (axis == Axis::z)
	{
		result(0, 1) = -sinTheta;
		result(1, 0) = sinTheta;
	}

	return (result);
}

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Matrix<R, C, T>& matrix)
{
	out << std::endl;

	for (int r = 0; r < R; r++)
	{
		for (int c = 0; c < C; c++)
		{
			out << matrix(r, c) << ", ";
		}
		out << std::endl;
	}

	return (out);
}