#pragma once

#include <iostream>
#include <initializer_list>
#include <stdint.h>
#include <stdexcept>
#include <array>
#include <type_traits>
#include <string>

/**
 * @file point.hpp
 * @brief Generic fixed-size point/vector class with math utilities.
 *
 * @details
 * Provides a templated Point class supporting 2D, 3D, and 4D points/vectors.
 * Supports arithmetic, indexing, rotation, normalization, and utility methods
 * such as dot and cross products. Constrained to integral and floating-point
 * element types.
 */

enum class Axis { x, y, z };

#define POINT_TEMPLATE template <ValidPointType T, size_t S> \
	requires ValidPointRange<S>

#define POINT_CAST_TEMPLATE template <ValidPointType CT, size_t CS> \
	requires ValidPointRange<CS>

#define POINT_INIT_TEMPLATE template <typename... Init> \
	requires (sizeof...(Init) <= S) && (ValidPointType<std::decay_t<Init>> && ...)

#define POINT_INIT_CAST_TEMPLATE template <ValidPointType CT, size_t CS, typename... Init> \
	requires (ValidPointRange<CS>) && (sizeof...(Init) <= S - CS) && (ValidPointType<std::decay_t<Init>> && ...)

template <typename T>
concept ValidPointType = (std::is_floating_point<T>().value || std::is_integral<T>().value);

template <size_t S>
concept ValidPointRange = (S > 1 && S <= 4);

/**
 * @brief Fixed-size point or vector of dimension @p S.
 *
 * @tparam T Element type (must be floating-point or integral).
 * @tparam S Number of dimensions (2–4).
 *
 * @details
 * Provides component accessors (x, y, z, w), arithmetic operators, rotation,
 * normalization, dot/cross products, and length computation. Can be constructed
 * from variadic values, other points (cast), or from strings.
 */
POINT_TEMPLATE
class Point
{
	private:
		std::array<T, S> data{};

	public:
		/** @brief Default constructor (zero-initialized). */
		Point();

		/**
		 * @brief Constructs a point from up to S values.
		 * @param init Values to initialize coordinates with.
		 * @note If initialized with 1 value, all coordinates will be initialized with that value.
		 * @note If initialized with more than 1 value, all other coordinates will be initialized with 0.
		 */
		POINT_INIT_TEMPLATE
		Point(Init... init);

		/**
		 * @brief Cast constructor from another point type/size.
		 * @tparam CT Source element type.
		 * @tparam CS Source dimension.
		 * @param other Point to cast from.
		 */
		POINT_CAST_TEMPLATE
		Point(const Point<CT, CS>& other);

		/**
		 * @brief Cast + initialize constructor.
		 * @tparam CT Source element type.
		 * @tparam CS Source dimension.
		 * @param other Point to cast from.
		 * @param init Extra values to fill remaining dimensions.
		 */
		POINT_INIT_CAST_TEMPLATE
		Point(const Point<CT, CS>& other, Init... init);

		/**
		 * @brief Assignment from another point (cross-type).
		 * @tparam CT Source element type.
		 * @tparam CS Source dimension.
		 * @param other Point to assign from.
		 * @return Reference to this point.
		 */
		POINT_CAST_TEMPLATE
		Point<T, S>& operator=(const Point<CT, CS>& other);

		/** @brief Destructor. */
		~Point();

		/** @name Component accessors (mutable) */
		///@{
		T& x() {return (data[0]);}
		T& y() {return (S > 1 ? data[1] : data[data.size() - 1]);}
		T& z() {return (S > 2 ? data[2] : data[data.size() - 1]);}
		T& w() {return (S > 3 ? data[3] : data[data.size() - 1]);}
		///@}

		/** @name Component accessors (const) */
		///@{
		const T& x() const {return (data[0]);}
		const T& y() const {return (S > 1 ? data[1] : data[data.size() - 1]);}
		const T& z() const {return (S > 2 ? data[2] : data[data.size() - 1]);}
		const T& w() const {return (S > 3 ? data[3] : data[data.size() - 1]);}
		///@}

		/**
		 * @brief Access coordinate by index.
		 * @param i Index (0 <= i < S).
		 * @return Reference to the element.
		 */
		T& operator[](const size_t i);
		const T& operator[](const size_t i) const; /**< @copydoc operator[] */

		/** @name Arithmetic operators (element-wise) */
		///@{
		Point<T, S> operator+(const Point<T, S>& other) const;
		Point<T, S> operator-(const Point<T, S>& other) const;
		Point<T, S> operator*(const Point<T, S>& other) const;
		Point<T, S> operator/(const Point<T, S>& other) const;
		
		void operator+=(const Point<T, S>& other);
		void operator-=(const Point<T, S>& other);
		void operator*=(const Point<T, S>& other);
		void operator/=(const Point<T, S>& other);
		///@}

		/**
		 * @brief Rotates the point (2D).
		 * @tparam PS Dimension (must be < 3).
		 * @param degrees Rotation angle in degrees.
		 */
		template <size_t PS = S> requires (PS < 3)
		void Rotate(const T& degrees);

		/**
		 * @brief Rotates the point around an axis (3D/4D).
		 * @tparam PS Dimension (must be > 2).
		 * @param degrees Rotation angle in degrees.
		 * @param axis Axis to rotate around.
		 */
		template <size_t PS = S> requires (PS > 2)
		void Rotate(const T& degrees, const Axis& axis);

		/**
		 * @brief Rotates the point by Euler angles (3D/4D).
		 * @tparam PS Dimension (must be > 2).
		 * @param rotation Euler angles (pitch, yaw, roll).
		 */
		template <size_t PS = S> requires (PS > 2)
		void Rotate(const Point<T, S>& rotation);

		/** @brief Normalizes the vector in place. */
		void Normalize();

		/** @brief Returns a normalized copy of the vector. */
		Point<T, S> Normalized() const;

		/** @brief Converts the vector to unit length in place. */
		void Unitize();

		/** @brief Returns a unit-length copy of the vector. */
		Point<T, S> Unitized() const;

		/**
		 * @brief Computes the Euclidean length (magnitude).
		 * @return Length of the vector.
		 */
		T Length() const;

		template <size_t PS = S> requires (PS == 2)
		float Angle() const;

		template <size_t PS = S> requires (PS == 3)
		Point<float, 2> Angles() const;

		/**
		 * @brief Computes the dot product of two points/vectors.
		 * @param a First vector.
		 * @param b Second vector.
		 * @return Dot product scalar.
		 */
		static T Dot(const Point<T, S>& a, const Point<T, S>& b);

		/**
		 * @brief Computes the cross product (3D/4D only).
		 * @tparam PS Dimension (must be > 2).
		 * @param a First vector.
		 * @param b Second vector.
		 * @return Cross product vector.
		 */
		template <size_t PS = S> requires (PS > 2)
		static Point<T, S> Cross(const Point<T, S>& a, const Point<T, S>& b);

		/**
		 * @brief Parses a point from a string.
		 * @param string Input string (e.g., "1.0,2.0,3.0").
		 * @return Constructed Point.
		 */
		static Point<T, S> FromString(const std::string& string);

		static Point<T, S> Rotation(const Point<T, S>& rotation);
};

POINT_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Point<T, S>& point);

typedef Point<float, 2> point2D;
typedef Point<float, 3> point3D;
typedef Point<float, 4> point4D;

typedef Point<double, 2> dpoint2D;
typedef Point<double, 3> dpoint3D;
typedef Point<double, 4> dpoint4D;

#include "point.tpp"