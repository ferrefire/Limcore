#include "matrix.hpp"

#include <stdexcept>

MATRIX_TEMPLATE
Matrix<R, C>::Matrix()
{

}

MATRIX_TEMPLATE
Matrix<R, C>::Matrix(std::array<float, R * C> init)
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
float& Matrix<R, C>::operator()(const uint32_t row, const uint32_t col)
{
	uint32_t i = col + (row * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
Matrix<R, C> Matrix<R, C>::operator+(Matrix<R, C> other)
{
	Matrix<R, C> result;

	for (int i = 0; i < size; i++)
	{
		result[i] = data[i] + other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C> Matrix<R, C>::operator-(Matrix<R, C> other)
{
	Matrix<R, C> result;

	for (int i = 0; i < size; i++)
	{
		result[i] = data[i] - other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C> Matrix<R, C>::operator*(Matrix<R, C> other)
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
void Matrix<R, C>::operator+=(Matrix<R, C> other)
{
	for (int i = 0; i < size; i++)
	{
		data[i] += other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C>::operator-=(Matrix<R, C> other)
{
	for (int i = 0; i < size; i++)
	{
		data[i] -= other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C>::operator*=(Matrix<R, C> other)
{
	*this = *this * other;
}

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, Matrix<R, C> matrix)
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