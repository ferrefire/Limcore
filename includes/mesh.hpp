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

#define MESH_TEMPLATE template <VertexConfig V, VkIndexType I>

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
		Mesh();
		~Mesh();

		void Create(Device* meshDevice = nullptr);
		void Create(const Shape<V, I>& shape, Device* meshDevice = nullptr);
		void Create(ModelLoader modelLoader, Device* meshDevice = nullptr);

		void Destroy();

		const std::vector<Vertex<V>>& GetVertices() const;
		const std::vector<indexType>& GetIndices() const;
		const std::vector<float>& GetData() const;

		void SetVertices(const std::vector<Vertex<V>>& newVertices);
		void AddVertex(Vertex<V> vertex);

		void SetIndices(const std::vector<indexType>& newIndices);
		void AddIndex(indexType index);

		void SetShape(const Shape<V, I>& shape);

		void Bind(VkCommandBuffer commandBuffer);

		VertexInfo GetVertexInfo();

		//static VertexInfo GetVertexInfo(VertexConfig config);
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