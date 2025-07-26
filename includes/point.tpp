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
	size_t i = 0;

	((data[i++] = static_cast<T>(init)), ...);

	if (sizeof...(Init) == 1) { for (i = 1; i < S; i++) { data[i] = data[0]; } }
}

POINT_TEMPLATE
POINT_CAST_TEMPLATE
Point<T, S>::Point(const Point<CT, CS>& other)
{
	const size_t size = (S < CS ? S : CS);

	for (int i = 0; i < size; i++) { this->data[i] = static_cast<T>(other[i]); }
}

POINT_TEMPLATE
POINT_INIT_CAST_TEMPLATE
Point<T, S>::Point(const Point<CT, CS>& other, Init... init)
{
	const size_t size = (S < CS ? S : CS);
	size_t i = 0;

	for (i = 0; i < size; i++) { this->data[i] = static_cast<T>(other[i]); }

	((data[i++] = static_cast<T>(init)), ...);
}

POINT_TEMPLATE
POINT_CAST_TEMPLATE
Point<T, S>& Point<T, S>::operator=(const Point<CT, CS>& other)
{
	const uint32_t size = (S < CS ? S : CS);

	for (size_t i = 0; i < size; i++) { this->data[i] = static_cast<T>(other[i]); }

	return (*this);
}

POINT_TEMPLATE
Point<T, S>::~Point()
{
	
}

POINT_TEMPLATE
T& Point<T, S>::operator[](const size_t i)
{
	if (i >= S) throw (std::out_of_range("Index out of bounds"));

	return (data[i]);
}

POINT_TEMPLATE
const T& Point<T, S>::operator[](const size_t i) const
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
	for (size_t i = 0; i < S; i++) { data[i] += other[i]; }
}

POINT_TEMPLATE
void Point<T, S>::operator-=(const Point<T, S>& other)
{
	for (size_t i = 0; i < S; i++) { data[i] -= other[i]; }
}

POINT_TEMPLATE
void Point<T, S>::operator*=(const Point<T, S>& other)
{
	for (size_t i = 0; i < S; i++) { data[i] *= other[i]; }
}

POINT_TEMPLATE
void Point<T, S>::operator/=(const Point<T, S>& other)
{
	for (size_t i = 0; i < S; i++) { data[i] /= other[i]; }
}

POINT_TEMPLATE
template <size_t PS> requires (PS < 3)
void Point<T, S>::Rotate(const T& degrees)
{
	const T radians = degrees * 0.0174532925;
	const T cosTheta = cos(radians);
	const T sinTheta = sin(radians);
	const Point<T, S> temp = *this;

	x() = (temp.x() * cosTheta) - (temp.y() * sinTheta);
	y() = (temp.x() * sinTheta) + (temp.y() * cosTheta);
}

POINT_TEMPLATE
template <size_t PS> requires (PS > 2)
void Point<T, S>::Rotate(const T& degrees, const Axis& axis)
{
	if (S > 3) return;

	const T radians = degrees * 0.0174532925;
	const T cosTheta = cos(radians);
	const T sinTheta = sin(radians);

	const size_t ai = (axis != Axis::x ? 0 : 1);
	const size_t bi = (axis != Axis::z ? 2 : 1);
	
	const T a = data[ai];
	const T b = data[bi];

	data[ai] = (a * cosTheta) - (b * sinTheta);
	data[bi] = (a * sinTheta) + (b * cosTheta);
}

POINT_TEMPLATE
void Point<T, S>::Normalize()
{
	T total = 0;

	for (size_t i = 0; i < S; i++) { total += fabs(data[i]); }

	if (total == 0) return;

	for (size_t i = 0; i < S; i++) { data[i] /= total; }
}

POINT_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Point<T, S>& point)
{
	for (size_t i = 0; i < S; i++)
	{
		out << static_cast<char>(119 + ((i + 1) % 4)) << ": " << point[i] << (i + 1 < S ? ", " : "");
	}

	return (out);
}