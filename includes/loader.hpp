#pragma once

#include "point.hpp"
#include "vertex.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <map>

#define CST(a) static_cast<size_t>(a)
#define C16(a) static_cast<uint16_t>(a)
#define C32(a) static_cast<uint32_t>(a)

enum class ModelType { None, Obj, Gltf };
enum class AttributeType { None, Float, Int };
enum class ImageType { None, Jpg, Png };

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

struct ImageInfo
{
	
};

class ByteReader
{
	private:
		const uint8_t* position;
		const uint8_t* end;

	public:
		ByteReader(const uint8_t* data, size_t size);

		size_t BytesLeft() const;
		size_t Offset(const uint8_t* base) const;

		uint8_t Read8();
		uint16_t Read16();
		uint32_t Read32();
		void Skip(size_t bytes);
};

class Loader
{
	private:
		static std::string GetValue(const std::string& content, const std::string& target);
		static AttributeInfo GetAttribute(const std::string& accessContent, const std::string& viewContent);
		static ModelInfo GetObjInfo(const std::string& name);
		static ModelInfo GetGltfInfo(const std::string& name);

	public:
		static ModelInfo GetModelInfo(const std::string& name, const ModelType& type);
		static ImageInfo GetImageInfo(const std::string& name, const ImageType& type);

		static void GetBytes(const std::string& name, char* address, size_t offset, size_t size);
};