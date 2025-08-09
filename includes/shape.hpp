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

#define SHAPE_TEMPLATE template <VertexConfig V, VkIndexType I>

enum class ShapeType { Quad, Cube };

SHAPE_TEMPLATE
class Shape
{
	using indexType = typename std::conditional_t<I == VK_INDEX_TYPE_UINT16, uint16_t, uint32_t>;
	static const bool hasIndices = I != VK_INDEX_TYPE_NONE_KHR;
	static const bool hasPosition = Bitmask::HasFlag(V, Position);
	static const bool hasCoordinate = Bitmask::HasFlag(V, Coordinate);
	static const bool hasNormal = Bitmask::HasFlag(V, Normal);
	static const bool hasColor = Bitmask::HasFlag(V, Color);

	private:
		std::vector<Vertex<V>> vertices;
		std::vector<indexType> indices;

		void CreateQuad();
		void CreateCube();

	public:
		Shape();
		Shape(ShapeType type);
		Shape(ModelLoader loader);
		~Shape();

		void Create(ShapeType type);
		void Create(ModelLoader loader);

		void Destroy();

		const std::vector<Vertex<V>>& GetVertices() const;
		const std::vector<indexType>& GetIndices() const;

		void Move(const point3D& translation);
		void Rotate(const float& degrees, const Axis& axis);
		void Centerize();
		void Scalarize();
		void Join(const Shape<V, I>& other, bool offset = true);
};

#include "shape.tpp"