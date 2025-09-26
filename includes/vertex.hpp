#pragma once

#include "point.hpp"
#include "bitmask.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

/**
 * @file vertex.hpp
 * @brief Vertex configuration and templated vertex structures.
 *
 * @details
 * Provides a flexible system for defining vertex layouts using a bitmask-based
 * configuration. Depending on the active bits, a templated @ref Vertex struct
 * includes position, normal, texture coordinates, and/or color attributes.
 * Includes helper structures and descriptions for Vulkan pipeline creation.
 */

#define VERTEX_TEMPLATE template <VertexConfig V>

/** @brief Bitmask flags for vertex layout configuration. */
typedef enum VertexConfigBits
{
	None = 0,
	Position = 1 << 0,
	Normal = 1 << 1,
	Coordinate = 1 << 2,
	Color = 1 << 3,
} VertexConfigBits;
typedef uint32_t VertexConfig; /**< @brief Bitmask type representing a vertex layout configuration. */

/**
 * @brief Describes a vertex input layout for Vulkan.
 *
 * @details
 * Holds binding and attribute descriptions used during graphics pipeline
 * creation. Also stores counts of bindings, attributes, and floats per vertex.
 */
struct VertexInfo
{
	uint32_t bindingCount = 0;
	uint32_t attributeCount = 0;
	uint32_t floatCount = 0;
	VkVertexInputBindingDescription bindingDescription{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

struct PositionStruct { point3D position; };
struct NormalStruct { point3D normal; };
struct CoordinateStruct { point2D coordinate; };
struct ColorStruct { point3D color; };

VERTEX_TEMPLATE
struct Empty {};

/**
 * @brief Generic vertex structure based on a configuration bitmask.
 *
 * @tparam V Vertex layout configuration (bitmask of @ref VertexConfigBits).
 *
 * @details
 * Uses template metaprogramming to include only the attributes specified
 * in the bitmask @p V. For example:
 * - @c Vertex<Position> -> includes position only.
 * - @code Vertex<Position | Normal> @endcode -> includes position and normal.
 * - @code Vertex<Position | Coordinate | Color> @endcode -> includes position, texcoord, and color.
 *
 * Provides a method to export vertex data as a flat float array
 * for use in buffer uploads.
 */
VERTEX_TEMPLATE
struct Vertex :
	std::conditional_t<Bitmask::HasFlag(V, Position), PositionStruct, Empty<Position>>,
	std::conditional_t<Bitmask::HasFlag(V, Normal), NormalStruct, Empty<Normal>>,
	std::conditional_t<Bitmask::HasFlag(V, Coordinate), CoordinateStruct, Empty<Coordinate>>,
	std::conditional_t<Bitmask::HasFlag(V, Color), ColorStruct, Empty<Color>>
{
	/**
	 * @brief Returns the vertex attributes as a flat float array.
	 * @return Vector of floats containing all active attributes in order.
	 */
	std::vector<float> GetData();
};

VERTEX_TEMPLATE
std::ostream& operator<<(std::ostream& out, Vertex<V> vertex);

#include "vertex.tpp"