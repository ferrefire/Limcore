#pragma once

#include "point.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

#define VERTEX_TEMPLATE template <VertexConfig V>

typedef enum VertexConfigBits
{
	None = 0,
	Position = 1 << 0,
	Coordinate = 1 << 1,
	Normal = 1 << 2,
	Color = 1 << 3,
} VertexConfigBits;
typedef uint32_t VertexConfig;

struct VertexInfo
{
	uint32_t bindingCount = 0;
	uint32_t attributeCount = 0;
	uint32_t floatCount = 0;
	VkVertexInputBindingDescription bindingDescription{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

struct PositionStruct { point3D position; };
struct CoordinateStruct { point2D coordinate; };
struct NormalStruct { point3D normal; };
struct ColorStruct { point3D color; };

VERTEX_TEMPLATE
struct Empty {};

VERTEX_TEMPLATE
struct Vertex :
	std::conditional_t<Bitmask::HasFlag(V, Position), PositionStruct, Empty<Position>>,
	std::conditional_t<Bitmask::HasFlag(V, Coordinate), CoordinateStruct, Empty<Coordinate>>,
	std::conditional_t<Bitmask::HasFlag(V, Normal), NormalStruct, Empty<Normal>>,
	std::conditional_t<Bitmask::HasFlag(V, Color), ColorStruct, Empty<Color>>
{
	std::vector<float> GetData();
};

VERTEX_TEMPLATE
std::ostream& operator<<(std::ostream& out, Vertex<V> vertex);

#include "vertex.tpp"