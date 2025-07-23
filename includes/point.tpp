#include "point.hpp"

#include <stdexcept>

POINT_TEMPLATE
Point<T, S>::Point()
{
	
}

POINT_TEMPLATE
Point<T, S>::Point(const T init)
{
	for (int i = 0; i < S; i++)
	{
		data[i] = init;
	}
}

POINT_TEMPLATE
Point<T, S>::Point(const std::array<T, S> init)
{
	for (int i = 0; i < S; i++)
	{
		data[i] = init[i];
	}
}

POINT_TEMPLATE
POINT_INIT_TEMPLATE
Point<T, S>::Point(Init&&... init)
{
	int i = 0;
	((data[i++] = std::forward<T>(init)), ...);
}

POINT_TEMPLATE
Point<T, S>& Point<T, S>::operator=(const Point<T, S>& other)
{
	this->data = other.data;
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
	for (int i = 0; i < S; i++) data[i] += other[i];
}

POINT_TEMPLATE
void Point<T, S>::operator-=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) data[i] -= other[i];
}

POINT_TEMPLATE
void Point<T, S>::operator*=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) data[i] *= other[i];
}

POINT_TEMPLATE
void Point<T, S>::operator/=(const Point<T, S>& other)
{
	for (int i = 0; i < S; i++) data[i] /= other[i];
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