#pragma once

#include "buffer.hpp"
#include "point.hpp"
#include "device.hpp"
#include "bitmask.hpp"
#include "vertex.hpp"
#include "shape.hpp"
#include "loader.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <variant>
#include <type_traits>
#include <iostream>
#include <string>

/**
 * @file mesh.hpp
 * @brief Templated mesh container and GPU upload helpers for Vulkan.
 *
 * @details
 * Provides a mesh class templated on a vertex layout (@ref VertexConfig) and index type
 * (@c VkIndexType). The class owns CPU-side vertex/index data and the corresponding GPU
 * buffers, and offers helpers to populate data from shapes or model loaders, and bind the
 * buffers for drawing.
 */

#define MESH_TEMPLATE template <VertexConfig V, VkIndexType I>

/**
 * @brief Geometry container and buffer manager for a specific vertex layout and index type.
 *
 * @tparam V Vertex layout bitmask (e.g., @c Position, @c Normal, @c Coordinate, @c Color).
 * @tparam I Vulkan index type (@c VK_INDEX_TYPE_UINT16, @c VK_INDEX_TYPE_UINT32, or @c VK_INDEX_TYPE_NONE_KHR).
 *
 * @details
 * Stores CPU-side vertex and index arrays (typed by @p V and @p I), creates matching
 * @ref Buffer GPU buffers, and binds them for rendering. Compile-time traits expose
 * whether the mesh has positions, normals, coordinates, colors, and indices.
 * 
 * Typical usage:
 * - Construct and call one of the @ref Create() overloads (empty, from @ref Shape, or loader).
 * - Optionally mutate with @ref SetVertices(), @ref SetIndices(), @ref AddVertex(), @ref AddIndex().
 * - Bind buffers with @ref Bind() during command recording.
 * - Destroy resources with @ref Destroy() when done.
 */
MESH_TEMPLATE
class Mesh
{
	using indexType = typename std::conditional_t<I == VK_INDEX_TYPE_UINT16, uint16_t, uint32_t>;
	static const bool hasIndices = I != VK_INDEX_TYPE_NONE_KHR;
	static const bool hasPosition = Bitmask::HasFlag(V, Position);
	static const bool hasNormal = Bitmask::HasFlag(V, Normal);
	static const bool hasCoordinate = Bitmask::HasFlag(V, Coordinate);
	static const bool hasColor = Bitmask::HasFlag(V, Color);

	private:
		Device* device = nullptr;

		std::vector<float> data;
		std::vector<Vertex<V>> vertices;
		std::vector<indexType> indices;

		Buffer vertexBuffer;
		Buffer indexBuffer;

		void CreateData();
		void CreateVertexBuffer();
		void CreateIndexBuffer();

	public:
		/** @brief Constructs an empty mesh (no GPU resources yet). */
		Mesh();

		/** @brief Destroys GPU buffers if still allocated. */
		~Mesh();

		/**
		 * @brief Creates GPU buffers for the current CPU-side data.
		 * @param meshDevice Device used to allocate and upload buffers; if @c nullptr, uses the stored device.
		 * @note Use this overload after calling @ref SetVertices() / @ref SetIndices().
		 */
		void Create(Device* meshDevice = nullptr);

		/**
		 * @brief Initializes the mesh from a procedural shape and uploads to GPU.
		 * @param shape Shape providing vertices (and indices if @c hasIndices).
		 * @param meshDevice Device used to allocate and upload buffers; if @c nullptr, uses the stored device.
		 */
		void Create(const Shape<V, I>& shape, Device* meshDevice = nullptr);

		/**
		 * @brief Initializes the mesh from a model loader and uploads to GPU.
		 * @param modelLoader Loader providing vertex/index data compatible with @p V/@p I.
		 * @param meshDevice Device used to allocate and upload buffers; if @c nullptr, uses the stored device.
		 */
		void Create(ModelLoader modelLoader, Device* meshDevice = nullptr);

		/** @brief Destroys GPU buffers and clears CPU-side data. */
		void Destroy();

		/**
		 * @brief Returns the typed vertex array.
		 * @return Const reference to the vertex vector.
		 */
		const std::vector<Vertex<V>>& GetVertices() const;

		/**
		 * @brief Returns the index array.
		 * @return Const reference to the index vector.
		 * @note Empty if @c hasIndices is false.
		 */
		const std::vector<indexType>& GetIndices() const;

		/**
		 * @brief Returns the interleaved vertex buffer data as raw floats.
		 * @return Const reference to the raw vertex data vector.
		 */
		const std::vector<float>& GetData() const;

		/**
		 * @brief Replaces the typed vertex array and rebuilds interleaved data.
		 * @param newVertices New vertex data.
		 * @note Call @ref Create() to re-upload to GPU after modifying vertices.
		 */
		void SetVertices(const std::vector<Vertex<V>>& newVertices);

		/**
		 * @brief Appends a single vertex to the typed array and updates interleaved data.
		 * @param vertex Vertex to add.
		 * @note Call @ref Create() to re-upload to GPU after modifying vertices.
		 */
		void AddVertex(Vertex<V> vertex);

		/**
		 * @brief Replaces the index array.
		 * @param newIndices New indices.
		 * @note Ignored if @c hasIndices is false. Call @ref Create() to re-upload to GPU.
		 */
		void SetIndices(const std::vector<indexType>& newIndices);

		/**
		 * @brief Appends a single index to the index array.
		 * @param index Index value to add.
		 * @note Ignored if @c hasIndices is false. Call @ref Create() to re-upload to GPU.
		 */
		void AddIndex(indexType index);

		/**
		 * @brief Replaces mesh data from a shape (vertices and indices) and rebuilds GPU buffers.
		 * @param shape Procedural shape to copy from.
		 */
		void SetShape(const Shape<V, I>& shape);

		/**
		 * @brief Binds the mesh's vertex (and index) buffers to a command buffer.
		 * @param commandBuffer Command buffer to record the bindings into.
		 * @note If @c hasIndices is true, also binds the index buffer with type @p I.
		 */
		void Bind(VkCommandBuffer commandBuffer);

		/**
		 * @brief Returns vertex input state information for pipeline creation.
		 * @return VertexInfo describing binding/attribute layouts for @p V.
		 */
		VertexInfo GetVertexInfo();
};

MESH_TEMPLATE
std::ostream& operator<<(std::ostream& out, const Mesh<V, I>& mesh);

typedef Mesh<Position, VK_INDEX_TYPE_UINT16> meshP16;
typedef Mesh<Position | Coordinate, VK_INDEX_TYPE_UINT16> meshPC16;
typedef Mesh<Position | Normal, VK_INDEX_TYPE_UINT16> meshPN16;
typedef Mesh<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> meshPNC16;
typedef Mesh<Position, VK_INDEX_TYPE_UINT32> meshP32;
typedef Mesh<Position | Coordinate, VK_INDEX_TYPE_UINT32> meshPC32;
typedef Mesh<Position | Normal, VK_INDEX_TYPE_UINT32> meshPN32;
typedef Mesh<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT32> meshPNC32;

#include "mesh.tpp"