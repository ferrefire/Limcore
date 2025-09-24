#pragma once

#include "point.hpp"

#include <iostream>
#include <stdint.h>
#include <array>

/**
 * @file matrix.hpp
 * @brief Generic fixed-size matrix class and math utilities.
 *
 * @details
 * Provides a templated Matrix class supporting 1x1 through 4x4 matrices,
 * constrained to floating-point element types (for now). Includes operators for
 * arithmetic, indexing, and multiplication with points, as well as
 * common transformation constructors (identity, scaling, translation,
 * rotation, projection, and view).
 */

#define MATRIX_TEMPLATE template <size_t R, size_t C, ValidMatrixType T> \
	requires (ValidMatrixRange<R> && ValidMatrixRange<C>)

#define MATRIX_CAST_TEMPLATE template <ValidMatrixType CT>

template <size_t S>
concept ValidMatrixRange = (S >= 1 && S <= 4);

template <typename T>
concept ValidMatrixType = (std::is_floating_point<T>().value);

/**
 * @brief Generic fixed-size matrix (R x C).
 *
 * @tparam R Number of rows (1–4).
 * @tparam C Number of columns (1–4).
 * @tparam T Floating-point element type (default: float).
 *
 * @details
 * Provides basic matrix arithmetic, element access, and transformation
 * operations commonly used in graphics (scale, translate, rotate, projection).
 */
template <size_t R, size_t C, ValidMatrixType T = float>
requires (ValidMatrixRange<R> && ValidMatrixRange<C>)
class Matrix
{
	static const size_t size = R * C;

	private:
		std::array<T, R * C> data{};

	public:
		/** @brief Default constructor (zero-initialized matrix). */
		Matrix();

		/**
		 * @brief Constructs a matrix from an array of values.
		 * @param init Array containing R*C elements.
		 */
		Matrix(const std::array<T, R * C>& init);

		/**
		 * @brief Constructs a matrix by casting from another type.
		 * @tparam CT Element type of the other matrix.
		 * @param other Matrix to cast from.
		 */
		MATRIX_CAST_TEMPLATE
		Matrix(const Matrix<R, C, CT>& other);

		/**
		 * @brief Assignment operator (same type).
		 * @param other Matrix to assign from.
		 * @return Reference to this matrix.
		 */
		Matrix<R, C, T>& operator=(const Matrix<R, C, T>& other);

		/**
		 * @brief Assignment operator (cross-type).
		 * @tparam CT Element type of the other matrix.
		 * @param other Matrix to assign from.
		 * @return Reference to this matrix.
		 */
		MATRIX_CAST_TEMPLATE
		Matrix<R, C, T>& operator=(const Matrix<R, C, CT>& other);
		
		/** @brief Destructor. */
		~Matrix();

		/**
		 * @brief Access an element by linear index.
		 * @param i Index (0 <= i < R*C).
		 * @return Reference to the element.
		 */
		T& operator[](const size_t i);

		/** @copydoc operator[] */
		const T& operator[](const size_t i) const;

		/**
		 * @brief Access an element by row/column.
		 * @param row Row index (0 <= row < R).
		 * @param col Column index (0 <= col < C).
		 * @return Reference to the element.
		 */
		T& operator()(const size_t row, const size_t col);

		/**
		 * @brief Access an element by row/column.
		 * @param row Row index (0 <= row < R).
		 * @param col Column index (0 <= col < C).
		 * @return Reference to the element.
		 */
		const T& operator()(const size_t row, const size_t col) const;

		/** @name Arithmetic operators */
		///@{
		Matrix<R, C, T> operator+(const Matrix<R, C, T>& other) const;
		Matrix<R, C, T> operator-(const Matrix<R, C, T>& other) const;
		Matrix<R, C, T> operator*(const Matrix<R, C, T>& other) const;
		Point<T, C> operator*(const Point<T, C>& point) const;

		void operator+=(const Matrix<R, C, T>& other);
		void operator-=(const Matrix<R, C, T>& other);
		void operator*=(const Matrix<R, C, T>& other);
		///@}

		/**
		 * @brief Scales the matrix by a vector.
		 * @param scalar Scaling factors per axis.
		 */
		void Scale(const Point<T, C>& scalar);

		/**
		 * @brief Translates the matrix by a vector.
		 * @param translation Translation vector.
		 */
		void Translate(const Point<T, C>& translation);

		/**
		 * @brief Applies a rotation around an axis.
		 * @param degrees Angle in degrees.
		 * @param axis Axis of rotation.
		 * @note Only valid for 4-column matrices.
		 */
		template <size_t CS = C> requires (CS == 4)
		void Rotate(const T& degrees, const Axis& axis);

		/**
		 * @brief Applies a rotation defined by Euler angles.
		 * @param rotation Pitch, yaw, roll vector.
		 * @note Only valid for 4-column matrices.
		 */
		template <size_t CS = C> requires (CS == 4)
		void Rotate(const point3D& rotation);

		/** @brief Returns an identity matrix. */
		static Matrix<R, C, T> Identity();

		/**
		 * @brief Returns a scaling matrix.
		 * @param scalar Scaling factors per axis.
		 */
		static Matrix<R, C, T> Scalar(const Point<T, C>& scalar);

		/**
		 * @brief Returns a translation matrix.
		 * @param translation Translation vector.
		 */
		static Matrix<R, C, T> Translation(const Point<T, C>& translation);

		/**
		 * @brief Returns a rotation matrix around an axis.
		 * @param degrees Angle in degrees.
		 * @param axis Axis of rotation.
		 */
		static Matrix<R, C, T> Rotation(const T& degrees, const Axis& axis);

		/**
		 * @brief Returns a projection matrix.
		 * @param fov Field of view angle in degrees.
		 * @param ratio Aspect ratio (width/height).
		 * @param near Near clipping plane distance.
		 * @param far Far clipping plane distance.
		 */
		static Matrix<R, C, T> Projection(const T& fov, const T& ratio, const T& near, const T& far);

		/**
		 * @brief Returns a view matrix from a direction and position.
		 * @param direction Forward direction vector.
		 * @param position Camera position.
		 */
		static Matrix<R, C, T> View(const Point<T, C>& direction, const Point<T, C>& position);

		/**
		 * @brief Returns a look-at view matrix.
		 * @param position Camera position.
		 * @param target Target point to look at.
		 * @param up Up vector.
		 */
		static Matrix<R, C, T> Look(const Point<T, C>& position, const Point<T, C>& target, const Point<T, C>& up);
};

MATRIX_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Matrix<R, C, T>& matrix);

typedef Matrix<3, 3, float> mat3;
typedef Matrix<4, 4, float> mat4;

typedef Matrix<3, 3, double> dmat3;
typedef Matrix<4, 4, double> dmat4;

#include "matrix.tpp"