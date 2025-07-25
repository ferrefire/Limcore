#include "matrix.hpp"

#include <stdexcept>
#include <cmath>

MATRIX_TEMPLATE
Matrix<R, C>::Matrix()
{

}

MATRIX_TEMPLATE
Matrix<R, C>::Matrix(const std::array<float, R * C>& init)
{
	for (int i = 0; i < size; i++)
	{
		int r = i / R;
		int c = i % R;

		(*this)(r, c) = init[i];
	}
}

MATRIX_TEMPLATE
Matrix<R, C>& Matrix<R, C>::operator=(const Matrix<R, C>& other)
{
	this->data = other.data;

	return (*this);
}

MATRIX_TEMPLATE
Matrix<R, C>::~Matrix()
{
	
}

MATRIX_TEMPLATE
float& Matrix<R, C>::operator[](const uint32_t i)
{
	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
const float& Matrix<R, C>::operator[](const uint32_t i) const
{
	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
const float& Matrix<R, C>::operator()(const uint32_t row, const uint32_t col) const
{
	uint32_t i = col + (row * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
float& Matrix<R, C>::operator()(const uint32_t row, const uint32_t col)
{
	uint32_t i = col + (row * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
Matrix<R, C> Matrix<R, C>::operator+(const Matrix<R, C>& other) const
{
	Matrix<R, C> result;

	for (int i = 0; i < size; i++)
	{
		result[i] = data[i] + other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C> Matrix<R, C>::operator-(const Matrix<R, C>& other) const
{
	Matrix<R, C> result;

	for (int i = 0; i < size; i++)
	{
		result[i] = data[i] - other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C> Matrix<R, C>::operator*(const Matrix<R, C>& other) const
{
	Matrix<R, C> result;

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
Point<float, C> Matrix<R, C>::operator*(const Point<float, C>& point) const
{
	Point<float, C> result;

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
void Matrix<R, C>::operator+=(const Matrix<R, C>& other)
{
	for (int i = 0; i < size; i++)
	{
		data[i] += other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C>::operator-=(const Matrix<R, C>& other)
{
	for (int i = 0; i < size; i++)
	{
		data[i] -= other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C>::operator*=(const Matrix<R, C>& other)
{
	*this = *this * other;
}

MATRIX_TEMPLATE
void Matrix<R, C>::Scale(const Point<float, C>& scalar)
{
	for (int i = 0; i < C; i++)
	{
		(*this)(i, i) *= scalar[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C>::Translate(const Point<float, C>& translation)
{
	for (int i = 0; i < C; i++)
	{
		(*this)(i, C - 1) += translation[i];
	}
}

MATRIX_TEMPLATE
template <uint32_t CS> requires (CS == 4)
void Matrix<R, C>::Rotate(const float& degrees, const Axis& axis)
{
	float radians = degrees * 0.0174532925;
	float cosTheta = cos(radians);
	float sinTheta = sin(radians);

	if (axis == Axis::x || axis == Axis::z) (*this)(1, 1) = cosTheta;
	if (axis == Axis::x || axis == Axis::y) (*this)(2, 2) = cosTheta;
	if (axis == Axis::y || axis == Axis::z) (*this)(0, 0) = cosTheta;

	if (axis == Axis::x)
	{
		(*this)(1, 2) = -sinTheta;
		(*this)(2, 1) = sinTheta;
	}
	else if (axis == Axis::y)
	{
		(*this)(0, 2) = -sinTheta;
		(*this)(2, 0) = sinTheta;
	}
	else if (axis == Axis::z)
	{
		(*this)(0, 1) = -sinTheta;
		(*this)(1, 0) = sinTheta;
	}
}

MATRIX_TEMPLATE
mat4 Matrix<R, C>::Identity()
{
	mat4 result({
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1});

	return (result);
}

MATRIX_TEMPLATE
mat4 Matrix<R, C>::Scalar(const Point<float, C>& scalar)
{
	mat4 result = mat4::Identity();

	result.Scale(scalar);

	return (result);
}

MATRIX_TEMPLATE
mat4 Matrix<R, C>::Translation(const Point<float, C>& translation)
{
	mat4 result = mat4::Identity();
	
	result.Translate(translation);

	return (result);
}

MATRIX_TEMPLATE
mat4 Matrix<R, C>::Rotation(const float& degrees, const Axis& axis)
{
	mat4 result = mat4::Identity();
	
	result.Rotate(degrees, axis);

	return (result);
}

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Matrix<R, C>& matrix)
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