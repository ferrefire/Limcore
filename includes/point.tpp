#include "point.hpp"

#include <stdexcept>
#include <cmath>

POINT_TEMPLATE
Point<T, S>::Point()
{
	
}

POINT_TEMPLATE
POINT_INIT_TEMPLATE
Point<T, S>::Point(Init... init)
{
	int i = 0;

	((data[i++] = init), ...);

	if (sizeof...(Init) == 1) { for (i = 1; i < S; i++) { data[i] = data[0]; } }
}

POINT_TEMPLATE
POINT_CAST_TEMPLATE
Point<T, S>::Point(const Point<T, CS>& other)
{
	uint32_t size = (S < CS ? S : CS);

	for (int i = 0; i < size; i++) { this->data[i] = other[i]; }
}

POINT_TEMPLATE
POINT_INIT_CAST_TEMPLATE
Point<T, S>::Point(const Point<T, CS>& other, Init... init)
{
	uint32_t size = (S < CS ? S : CS);
	int i = 0;

	for (i = 0; i < size; i++) { this->data[i] = other[i]; }

	((data[i++] = init), ...);
}

POINT_TEMPLATE
POINT_CAST_TEMPLATE
Point<T, S>& Point<T, S>::operator=(const Point<T, CS>& other)
{
	uint32_t size = (S < CS ? S : CS);

	for (int i = 0; i < size; i++) { this->data[i] = other[i]; }

	return (*this);
}

POINT_TEMPLATE
Point<T, S>::~Point()
{
	
}

POINT_TEMPLATE
T& Point<T, S>::operator[](const uint32_t i)
{
	if (i >= S) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

POINT_TEMPLATE
const T& Point<T, S>::operator[](const uint32_t i) const
{
	if (i >= S) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator+(const Point<T, S>& other) const
{
	Point<T, S> result;
	result += *this;
	result += other;

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator-(const Point<T, S>& other) const
{
	Point<T, S> result;
	result -= *this;
	result -= other;

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator*(const Point<T, S>& other) const
{
	Point<T, S> result;
	result *= *this;
	result *= other;

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator/(const Point<T, S>& other) const
{
	Point<T, S> result;
	result /= *this;
	result /= other;

	return (result);
}

POINT_TEMPLATE
void Point<T, S>::operator+=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) { data[i] += other[i]; }
}

POINT_TEMPLATE
void Point<T, S>::operator-=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) { data[i] -= other[i]; }
}

POINT_TEMPLATE
void Point<T, S>::operator*=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) { data[i] *= other[i]; }
}

POINT_TEMPLATE
void Point<T, S>::operator/=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) { data[i] /= other[i]; }
}

POINT_TEMPLATE
template <uint32_t PS> requires (PS < 3)
void Point<T, S>::Rotate(const float& degrees)
{
	float radians = degrees * 0.0174532925;
	float cosTheta = cos(radians);
	float sinTheta = sin(radians);
	Point<T, S> temp = *this;

	x() = (temp.x() * cosTheta) - (temp.y() * sinTheta);
	y() = (temp.x() * sinTheta) + (temp.y() * cosTheta);
}

POINT_TEMPLATE
template <uint32_t PS> requires (PS > 2)
void Point<T, S>::Rotate(const float& degrees, const Axis& axis)
{
	if (S > 3) return;

	T radians = degrees * 0.0174532925;
	T cosTheta = cos(radians);
	T sinTheta = sin(radians);

	int ai = (axis != Axis::x ? 0 : 1);
	int bi = (axis != Axis::z ? 2 : 1);
	
	T a = data[ai];
	T b = data[bi];

	data[ai] = (a * cosTheta) - (b * sinTheta);
	data[bi] = (a * sinTheta) + (b * cosTheta);
}

POINT_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Point<T, S>& point)
{
	for (int i = 0; i < S; i++)
	{
		out << static_cast<char>(119 + ((i + 1) % 4)) << ": " << point[i] << (i + 1 < S ? ", " : "");
	}

	return (out);
}