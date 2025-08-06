#pragma once

#include "point.hpp"
#include "vertex.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <map>

enum class ModelType { None, Obj, Gltf };
enum class AttributeType { None, Float, Int };

struct AttributeInfo
{
	VertexConfigBits type = None;
	int index = -1;
	size_t count = 0;
	AttributeType component = AttributeType::None;
	size_t dimensions = 0;
	std::pair<size_t, size_t> range;
};

struct IndexInfo
{
	VkIndexType type = VK_INDEX_TYPE_NONE_KHR;
	int index = -1;
	size_t count = 0;
	std::pair<size_t, size_t> range;
};

struct ModelInfo
{
	VertexConfig vertexConfig = None;
	IndexInfo indexInfo{};
	std::map<VertexConfigBits, AttributeInfo> attributes;
};

class Loader
{
	private:


	public:
		static ModelInfo GetModelInfo(const std::string& name, const ModelType& type);
		static ModelInfo GetObjInfo(const std::string& name);
		static ModelInfo GetGltfInfo(const std::string& name);

		//static ModelData LoadModel(const std::string& name, const ModelType& type);
		//static ModelData LoadObj(const std::string& name);
		//static ModelData LoadGltf(const std::string& name);
};