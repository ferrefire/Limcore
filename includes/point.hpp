#pragma once

#include <iostream>
#include <initializer_list>
#include <stdint.h>
#include <stdexcept>
#include <array>

#define POINT_TEMPLATE template <ValidType T, uint32_t S> \
	requires ValidRange<S>

template <typename T>
concept ValidType = (std::is_floating_point<T>().value || std::is_integral<T>().value);

template <uint32_t S>
concept ValidRange = (S >= 1 && S <= 4);

POINT_TEMPLATE
class Point
{
	private:
		std::array<T, S> data{};

	public:
		Point();
		Point(const T init);
		Point(std::initializer_list<T> init);
		Point<T, S>& operator=(const Point<T, S>& other);
		~Point();

		T& x() {return (data[0]);}
		T& y() {return (S > 1 ? data[1] : data[data.size() - 1]);}
		T& z() {return (S > 2 ? data[2] : data[data.size() - 1]);}
		T& w() {return (S > 3 ? data[3] : data[data.size() - 1]);}

		const T& x() const {return (data[0]);}
		const T& y() const {return (S > 1 ? data[1] : data[data.size() - 1]);}
		const T& z() const {return (S > 2 ? data[2] : data[data.size() - 1]);}
		const T& w() const {return (S > 3 ? data[3] : data[data.size() - 1]);}

		T& operator[](const uint32_t i);
		Point<T, S> operator+(Point<T, S> other);
		Point<T, S> operator-(Point<T, S> other);
		Point<T, S> operator*(Point<T, S> other);
		Point<T, S> operator/(Point<T, S> other);
		void operator+=(Point<T, S> other);
		void operator-=(Point<T, S> other);
		void operator*=(Point<T, S> other);
		void operator/=(Point<T, S> other);
};

//POINT_TEMPLATE
//std::ostream& operator<<(std::ostream& out, Point<T, S>& point);

POINT_TEMPLATE
std::ostream& operator<<(std::ostream& out, Point<T, S> point);

#include "point.tpp"

typedef Point<float, 2> point2D;
typedef Point<float, 3> point3D;
typedef Point<float, 4> point4D;

typedef Point<double, 2> dpoint2D;
typedef Point<double, 3> dpoint3D;
typedef Point<double, 4> dpoint4D;