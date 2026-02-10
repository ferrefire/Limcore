#pragma once

#include "point.hpp"
#include "vertex.hpp"
#include "bitmask.hpp"
#include "loader.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <string>

/**
 * @file shape.hpp
 * @brief Procedural and loaded shapes for templated vertex/index formats.
 *
 * @details
 * Provides a templated Shape class that generates primitives (quad, cube)
 * or builds geometry from a ModelLoader, for an arbitrary vertex layout
 * (@ref VertexConfig) and index type (@c VkIndexType). Shapes store CPU-side
 * vertices and indices and expose utilities for basic geometric transforms.
 */

#define SHAPE_TEMPLATE template <VertexConfig V, VkIndexType I>

enum class ShapeType { Quad, Plane, Cube, Cylinder };

struct ShapeSettings
{
	bool scalarized = true;
	Point<int, 2> resolution = {1, 1}; 
};

/**
 * @brief Geometry builder/container for a specific vertex layout and index type.
 *
 * @tparam V Vertex layout bitmask (e.g., @c Position, @c Normal, @c Coordinate, @c Color).
 * @tparam I Vulkan index type (@c VK_INDEX_TYPE_UINT16, @c VK_INDEX_TYPE_UINT32, or @c VK_INDEX_TYPE_NONE_KHR).
 *
 * @details
 * Holds typed vertices and indices in CPU memory. Can procedurally create a quad
 * or cube, or import geometry from a @c ModelLoader. Compile-time traits expose
 * whether the layout includes positions, normals, UVs, colors, and whether indices
 * are used. Includes utilities for translation, rotation, centering, scalarization,
 * and joining geometry.
 */
SHAPE_TEMPLATE
class Shape
{
	using indexType = typename std::conditional_t<I == VK_INDEX_TYPE_UINT16, uint16_t, uint32_t>;
	static const bool hasIndices = I != VK_INDEX_TYPE_NONE_KHR;
	static const bool hasPosition = Bitmask::HasFlag(V, Position);
	static const bool hasNormal = Bitmask::HasFlag(V, Normal);
	static const bool hasCoordinate = Bitmask::HasFlag(V, Coordinate);
	static const bool hasColor = Bitmask::HasFlag(V, Color);

	private:
		std::vector<Vertex<V>> vertices;
		std::vector<indexType> indices;

		ShapeSettings settings{};

		void CreateQuad();
		void CreatePlane();
		void CreateCube();

		template <VertexConfig PV = V, VkIndexType PI = I> requires (Bitmask::HasFlag(PV, Position)) && (PI != VK_INDEX_TYPE_NONE_KHR)
		void CreateCylinder();

	public:
		/** @brief Constructs an empty shape (no vertices/indices). */
		Shape();

		/**
		 * @brief Constructs and initializes a primitive shape.
		 * @param type Primitive type to build (quad or cube).
		 */
		Shape(ShapeType type, ShapeSettings shapeSettings = ShapeSettings{});

		/**
		 * @brief Constructs geometry from a model loader.
		 * @param loader Model loader providing geometry.
		 * @param scalarized If true, flattens/expands attributes to match layout @p V.
		 */
		Shape(ModelLoader loader, ShapeSettings shapeSettings = ShapeSettings{});

		/** @brief Destroys the shape and clears geometry. */
		~Shape();

		/**
		 * @brief Creates a primitive shape.
		 * @param type Primitive type to build.
		 */
		void Create(ShapeType type, ShapeSettings shapeSettings = ShapeSettings{});

		/**
		 * @brief Creates geometry from a model loader.
		 * @param loader Model loader providing geometry.
		 * @param scalarized If true, flattens/expands attributes so that the geometry has a scale of 1.
		 */
		void Create(ModelLoader loader, ShapeSettings shapeSettings = ShapeSettings{});

		/** @brief Clears vertices and indices. */
		void Destroy();

		/**
		 * @brief Accesses the typed vertex array.
		 * @return Const reference to the vertex vector.
		 */
		const std::vector<Vertex<V>>& GetVertices() const;

		/**
		 * @brief Accesses the index array.
		 * @return Const reference to the index vector (may be empty).
		 */
		const std::vector<indexType>& GetIndices() const;

		/**
		 * @brief Translates all vertices by a vector.
		 * @param translation Translation vector in object space.
		 */
		void Move(const point3D& translation);

		/**
		 * @brief Rotates all vertices around an axis.
		 * @param degrees Angle in degrees.
		 * @param axis Axis to rotate around.
		 */
		void Rotate(const float& degrees, const Axis& axis);

		void Scale(const point3D& scalar, bool scaleUV = false);

		/**
		 * @brief Recenters geometry around the origin.
		 * @details Typically subtracts the centroid or AABB center from all vertices.
		 */
		void Centerize();

		/**
		 * @brief Scales geometry so that it has a scale of 1.
		 * @details Flattens/expands attributes so that the geometry has a scale of 1.
		 */
		void Scalarize();

		/**
		 * @brief Appends geometry from another shape to this one.
		 * @param other Source shape to join.
		 * @param offset If true, offset indices to account for current vertex count.
		 */
		void Join(const Shape<V, I>& other, bool offset = true);
};

typedef Shape<Position, VK_INDEX_TYPE_UINT16> shapeP16;
typedef Shape<Position | Coordinate, VK_INDEX_TYPE_UINT16> shapePC16;
typedef Shape<Position | Normal, VK_INDEX_TYPE_UINT16> shapePN16;
typedef Shape<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> shapePNC16;
typedef Shape<Position, VK_INDEX_TYPE_UINT32> shapeP32;
typedef Shape<Position | Coordinate, VK_INDEX_TYPE_UINT32> shapePC32;
typedef Shape<Position | Normal, VK_INDEX_TYPE_UINT32> shapePN32;
typedef Shape<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT32> shapePNC32;

#include "shape.tpp"