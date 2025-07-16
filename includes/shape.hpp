#pragma once

#include "point.hpp"
#include "vertex.hpp"
#include "bitmask.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#define SHAPE_TEMPLATE template <VertexConfig V, VkIndexType I>

enum class ShapeType { Quad, Cube };

SHAPE_TEMPLATE
class Shape
{
	using indexType = typename std::conditional_t<I == VK_INDEX_TYPE_UINT16, uint16_t, uint32_t>;
	const bool hasIndices = I != VK_INDEX_TYPE_NONE_KHR;
	const bool hasPosition = Bitmask::HasFlag(V, Position);
	const bool hasCoordinate = Bitmask::HasFlag(V, Coordinate);
	const bool hasNormal = Bitmask::HasFlag(V, Normal);
	const bool hasColor = Bitmask::HasFlag(V, Color);

	private:
		std::vector<Vertex<V>> vertices;
		std::vector<indexType> indices;

		void CreateQuad();
		void CreateCube();

	public:
		Shape();
		Shape(ShapeType type);
		~Shape();

		void Create(ShapeType type);

		const std::vector<Vertex<V>>& GetVertices() const;
		const std::vector<indexType>& GetIndices() const;
};

#include "shape.tpp"