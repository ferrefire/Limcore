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
	for (size_t i = 0; i < size; i++)
	{
		size_t r = i / R;
		size_t c = i % R;

		(*this)(r, c) = init[i];
	}
}



MATRIX_TEMPLATE
MATRIX_CAST_TEMPLATE
Matrix<R, C, T>::Matrix(const Matrix<R, C, CT>& other)
{
	for (size_t i = 0; i < size; i++) { this->data[i] = static_cast<T>(other[i]); }
}

MATRIX_TEMPLATE
Matrix<R, C, T>& Matrix<R, C, T>::operator=(const Matrix<R, C, T>& other)
{
	for (size_t i = 0; i < size; i++) { this->data[i] = other[i]; }

	return (*this);
}

MATRIX_TEMPLATE
MATRIX_CAST_TEMPLATE
Matrix<R, C, T>& Matrix<R, C, T>::operator=(const Matrix<R, C, CT>& other)
{
	for (size_t i = 0; i < size; i++) { this->data[i] = static_cast<T>(other[i]); }

	return (*this);
}

MATRIX_TEMPLATE
Matrix<R, C, T>::~Matrix()
{
	
}

MATRIX_TEMPLATE
T& Matrix<R, C, T>::operator[](const size_t i)
{
	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
const T& Matrix<R, C, T>::operator[](const size_t i) const
{
	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
const T& Matrix<R, C, T>::operator()(const size_t row, const size_t col) const
{
	const size_t i = row + (col * R);

	if (i >= size) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

MATRIX_TEMPLATE
T& Matrix<R, C, T>::operator()(const size_t row, const size_t col)
{
	const size_t i = row + (col * R);

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

	for (size_t i = 0; i < size; i++)
	{
		result[i] = data[i] - other[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::operator*(const Matrix<R, C, T>& other) const
{
	Matrix<R, C, T> result;

	for (size_t r = 0; r < R; r++)
	{
		for (size_t c = 0; c < C; c++)
		{
			for (size_t i = 0; i < C; i++)
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

	for (size_t r = 0; r < R; r++)
	{
		for (size_t c = 0; c < C; c++)
		{
			result[r] += (*this)(r, c) * point[c];
		}
	}

	return (result);
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::operator+=(const Matrix<R, C, T>& other)
{
	for (size_t i = 0; i < size; i++)
	{
		data[i] += other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::operator-=(const Matrix<R, C, T>& other)
{
	for (size_t i = 0; i < size; i++)
	{
		data[i] -= other[i];
	}
}

MATRIX_TEMPLATE
void Matrix<R, C, T>::operator*=(const Matrix<R, C, T>& other)
{
	(*this) = (*this) * other;
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
template <size_t CS> requires (CS == 4)
void Matrix<R, C, T>::Rotate(const T& degrees, const Axis& axis)
{
	(*this) = Matrix<R, C, T>::Rotation(degrees, axis) * (*this);
}

MATRIX_TEMPLATE
template <size_t CS> requires (CS == 4)
void Matrix<R, C, T>::Rotate(const point3D& rotation)
{
	if (rotation.x() != 0) (*this) = Rotation(rotation.x(), Axis::x) * (*this);
	if (rotation.y() != 0) (*this) = Rotation(rotation.y(), Axis::y) * (*this);
	if (rotation.z() != 0) (*this) = Rotation(rotation.z(), Axis::z) * (*this);
}

MATRIX_TEMPLATE
template <size_t CS> requires (CS == 4)
Matrix<R, C, T> Matrix<R, C, T>::Inversed()
{
	const T a00 = (*this)(0, 0), a01 = (*this)(0, 1), a02 = (*this)(0, 2), a03 = (*this)(0, 3);
	const T a10 = (*this)(1, 0), a11 = (*this)(1, 1), a12 = (*this)(1, 2), a13 = (*this)(1, 3);
	const T a20 = (*this)(2, 0), a21 = (*this)(2, 1), a22 = (*this)(2, 2), a23 = (*this)(2, 3);
	const T a30 = (*this)(3, 0), a31 = (*this)(3, 1), a32 = (*this)(3, 2), a33 = (*this)(3, 3);

	const T b00 = a00 * a11 - a01 * a10;
	const T b01 = a00 * a12 - a02 * a10;
	const T b02 = a00 * a13 - a03 * a10;
	const T b03 = a01 * a12 - a02 * a11;
	const T b04 = a01 * a13 - a03 * a11;
	const T b05 = a02 * a13 - a03 * a12;
	const T b06 = a20 * a31 - a21 * a30;
	const T b07 = a20 * a32 - a22 * a30;
	const T b08 = a20 * a33 - a23 * a30;
	const T b09 = a21 * a32 - a22 * a31;
	const T b10 = a21 * a33 - a23 * a31;
	const T b11 = a22 * a33 - a23 * a32;

	const T det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

	const T invDet = 1.0 / det;

	Matrix<R, C, T> result;

	result(0, 0) = ( a11 * b11 - a12 * b10 + a13 * b09) * invDet;
	result(0, 1) = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
	result(0, 2) = ( a31 * b05 - a32 * b04 + a33 * b03) * invDet;
	result(0, 3) = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;

	result(1, 0) = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
	result(1, 1) = ( a00 * b11 - a02 * b08 + a03 * b07) * invDet;
	result(1, 2) = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
	result(1, 3) = ( a20 * b05 - a22 * b02 + a23 * b01) * invDet;

	result(2, 0) = ( a10 * b10 - a11 * b08 + a13 * b06) * invDet;
	result(2, 1) = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
	result(2, 2) = ( a30 * b04 - a31 * b02 + a33 * b00) * invDet;
	result(2, 3) = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;

	result(3, 0) = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
	result(3, 1) = ( a00 * b09 - a01 * b07 + a02 * b06) * invDet;
	result(3, 2) = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
	result(3, 3) = ( a20 * b03 - a21 * b01 + a22 * b00) * invDet;

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Identity()
{
	Matrix<R, C, T> result;

	for (size_t r = 0; r < R; r++)
	{
		for (size_t c = 0; c < C; c++)
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

	for (size_t i = 0; i < C; i++)
	{
		result(i, i) *= scalar[i];
	}

	result(C - 1, C - 1) = 1.0;

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Translation(const Point<T, C>& translation)
{
	Matrix<R, C, T> result = Identity();
	
	for (size_t i = 0; i < C; i++)
	{
		result(i, C - 1) += translation[i];
	}

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Rotation(const T& degrees, const Axis& axis)
{
	Matrix<R, C, T> result = Identity();
	
	const T radians = degrees * 0.0174532925;
	const T cosTheta = cos(radians);
	const T sinTheta = sin(radians);

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
Matrix<R, C, T> Matrix<R, C, T>::Projection(const T& fov, const T& ratio, const T& near, const T& far)
{
	Matrix<R, C, T> result;

	const T radians = (fov * 0.5) * 0.0174532925;
	const T tangent = tan(radians);
	const T top = near * tangent;
	const T right = top * ratio;

	result(0, 0) = near / right;
	result(1, 1) = near / top;
	result(2, 2) = far / (far - near);
	result(3, 2) = 1;
	result(2, 3) = (far * -near) / (far - near);
	result(3, 3) = 0;

	result(1, 1) *= -1;

	/*const T tangent = tan((fov * 0.0174532925) * 0.5);

	result(0, 0) = 1.0 / (ratio * tangent);
	result(1, 1) = 1.0 / tangent;
	result(2, 2) = far / (far - near);
	result(3, 2) = 1;
	result(2, 3) = (-near * far) / (far - near);*/

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Orthographic(const T& left, const T& right, const T& bottom, const T& top, const T& near, const T& far)
{
	Matrix<R, C, T> result;

	result(0, 0) = 2.0 / (right - left);
	result(1, 1) = 2.0 / (top - bottom);
	result(2, 2) = 1.0 / (far - near);
	result(3, 0) = -(right + left) / (right - left);
	result(3, 1) = -(top + bottom) / (top - bottom);
	result(3, 2) = -near / (far - near);
	result(3, 3) = 1.0;

	result(1, 1) *= -1;

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::View(const Point<T, C>& rotation, const Point<T, C>& position)
{
	Matrix<R, C, T> result = Identity();

	result.Rotate(rotation);
	result.Translate(position * point3D(-1));

	return (result);
}

MATRIX_TEMPLATE
Matrix<R, C, T> Matrix<R, C, T>::Look(const Point<T, C>& position, const Point<T, C>& target, const Point<T, C>& up)
{
	Matrix<R, C, T> result = Identity();

	const point3D foward = (target - position).Unitized();
	const point3D horizontal = point3D::Cross(up, foward).Unitized();
	const point3D vertical = point3D::Cross(foward, horizontal);

	result(0, 0) = horizontal.x();
	result(0, 1) = horizontal.y();
	result(0, 2) = horizontal.z();
	result(1, 0) = vertical.x();
	result(1, 1) = vertical.y();
	result(1, 2) = vertical.z();
	result(2, 0) = foward.x();
	result(2, 1) = foward.y();
	result(2, 2) = foward.z();
	result(0, 3) = -point3D::Dot(horizontal, position);
	result(1, 3) = -point3D::Dot(vertical, position);
	result(2, 3) = -point3D::Dot(foward, position);

	return (result);
}

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Matrix<R, C, T>& matrix)
{
	out << std::endl;

	for (size_t r = 0; r < R; r++)
	{
		for (size_t c = 0; c < C; c++)
		{
			out << matrix(r, c) << ", ";
		}
		out << std::endl;
	}

	return (out);
}