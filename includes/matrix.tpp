#include "matrix.hpp"

#include <stdexcept>

MATRIX_TEMPLATE
Matrix<R, C>::Matrix()
{

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
	uint32_t i = row + (col * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
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
	//Matrix<R, C> result;
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