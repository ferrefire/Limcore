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
	std::string accessContent = "";
	std::string viewContent = "";

	std::string viewIndex;
	std::string bufferIndex;

	std::string count;
	std::string component;
	std::string type;
	std::string buffer;
	std::string length;
	std::string offset;

	size_t Count() { return (std::stoul(count)); }
	size_t Offset() { return (std::stoul(offset)); }
	size_t Length() { return (std::stoul(length)); }
};

struct ModelInfo
{
	std::string name = "";
	size_t size = 0;
	VertexConfig vertexConfig = None;
	VkIndexType indexConfig = VK_INDEX_TYPE_NONE_KHR;
	AttributeInfo indexInfo{};
	std::map<VertexConfigBits, AttributeInfo> attributes;
};

class Loader
{
	private:
		static std::string GetValue(const std::string& content, const std::string& target);
		static AttributeInfo GetAttribute(const std::string& accessContent, const std::string& viewContent);

	public:
		static ModelInfo GetModelInfo(const std::string& name, const ModelType& type);
		static ModelInfo GetObjInfo(const std::string& name);
		static ModelInfo GetGltfInfo(const std::string& name);

		static void GetBytes(const std::string& name, char* address, size_t offset, size_t size);
};