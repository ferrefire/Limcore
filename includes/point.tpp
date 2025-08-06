#include "point.hpp"

#include <stdexcept>
#include <cmath>

#include "utilities.hpp"

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
	result = (*this);
	result += other;

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator-(const Point<T, S>& other) const
{
	Point<T, S> result;
	result = (*this);
	result -= other;

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator*(const Point<T, S>& other) const
{
	Point<T, S> result;
	result = (*this);
	result *= other;

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::operator/(const Point<T, S>& other) const
{
	Point<T, S> result;
	result = (*this);
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
	const Point<T, S> temp = (*this);

	x() = (temp.x() * cosTheta) - (temp.y() * sinTheta);
	y() = (temp.x() * sinTheta) + (temp.y() * cosTheta);
}

POINT_TEMPLATE
template <size_t PS> requires (PS > 2)
void Point<T, S>::Rotate(const T& degrees, const Axis& axis)
{
	if (S > 3 || degrees == 0) return;

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
template <size_t PS> requires (PS > 2)
void Point<T, S>::Rotate(const Point<T, S>& rotation)
{
	if (S > 3) return;

	if (rotation.x() != 0) Rotate(rotation.x(), Axis::x);
	if (rotation.y() != 0) Rotate(rotation.y(), Axis::y);
	if (rotation.z() != 0) Rotate(rotation.z(), Axis::z);
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
Point<T, S> Point<T, S>::Normalized() const
{
	Point<T, S> result = (*this);
	result.Normalize();

	return (result);
}

POINT_TEMPLATE
void Point<T, S>::Unitize()
{
	T length = Length();

	if (length == 0) return;

	for (size_t i = 0; i < S; i++) { data[i] /= length; }
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::Unitized() const
{
	Point<T, S> result = (*this);
	result.Unitize();

	return (result);
}

POINT_TEMPLATE
T Point<T, S>::Length() const
{
	T result = 0;

	for (size_t i = 0; i < S; i++) { result += data[i] * data[i]; }

	result = sqrt(result);

	return (result);
}

POINT_TEMPLATE
T Point<T, S>::Dot(const Point<T, S>& a, const Point<T, S>& b)
{
	T result = 0;

	for (size_t i = 0; i < S; i++) { result += a[i] * b[i]; }

	return (result);
}

POINT_TEMPLATE
template <size_t PS> requires (PS > 2)
Point<T, S> Point<T, S>::Cross(const Point<T, S>& a, const Point<T, S>& b)
{
	point3D result;

	result.x() = (a.y() * b.z()) - (a.z() * b.y());
	result.y() = (a.z() * b.x()) - (a.x() * b.z());
	result.z() = (a.x() * b.y()) - (a.y() * b.x());

	return (result);
}

POINT_TEMPLATE
Point<T, S> Point<T, S>::FromString(const std::string& string)
{
	Point<T, S> result;

	std::vector<std::string> values = Utilities::Split(string, " ");
	

	for (int i = 0; i < std::min(S, values.size()); i++)
	{
		result[i] = static_cast<T>(std::stof(values[i]));
	}

	return (result);
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