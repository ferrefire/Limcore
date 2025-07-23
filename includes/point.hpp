#pragma once

#include <iostream>
#include <initializer_list>
#include <stdint.h>
#include <stdexcept>
#include <array>
#include <type_traits>

#define POINT_TEMPLATE template <ValidPointType T, uint32_t S> \
	requires ValidPointRange<S>

#define POINT_INIT_TEMPLATE template <typename... Init> \
	requires (sizeof...(Init) <= S) && (ValidPointType<std::decay_t<Init>> && ...)

template <typename T>
concept ValidPointType = (std::is_floating_point<T>().value || std::is_integral<T>().value);

template <uint32_t S>
concept ValidPointRange = (S >= 1 && S <= 4);

POINT_TEMPLATE
class Point
{
	private:
		std::array<T, S> data{};

	public:
		Point();
		Point(const T init);
		Point(const std::array<T, S> init);
		POINT_INIT_TEMPLATE
		Point(Init&&... init);
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
		const T& operator[](const uint32_t i) const;

		Point<T, S> operator+(const Point<T, S>& other) const;
		Point<T, S> operator-(const Point<T, S>& other) const;
		Point<T, S> operator*(const Point<T, S>& other) const;
		Point<T, S> operator/(const Point<T, S>& other) const;
		void operator+=(const Point<T, S>& other);
		void operator-=(const Point<T, S>& other);
		void operator*=(const Point<T, S>& other);
		void operator/=(const Point<T, S>& other);
};

POINT_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Point<T, S>& point);

#include "point.tpp"

typedef Point<float, 2> point2D;
typedef Point<float, 3> point3D;
typedef Point<float, 4> point4D;

typedef Point<double, 2> dpoint2D;
typedef Point<double, 3> dpoint3D;
typedef Point<double, 4> dpoint4D;