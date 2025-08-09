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
enum class AttributeType { None, Position, Normal, Coordinate, Color, Index };
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
	std::string translation = "0, 0, 0";

	size_t Count() const { return (std::stoul(count)); }
	size_t Offset() const { return (std::stoul(offset)); }
	size_t Length() const { return (std::stoul(length)); }
	point3D Translation() const;
};

struct ModelInfo
{
	std::string name = "";
	ModelType type = ModelType::None;
	size_t ID = 0;
	size_t count = 0;
	size_t size = 0;
	VertexConfig vertexConfig = None;
	VkIndexType indexConfig = VK_INDEX_TYPE_NONE_KHR;
	std::map<AttributeType, AttributeInfo> attributes;

	const AttributeInfo& GetAttribute(const AttributeType& type) const;
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

class ModelLoader
{
	private:
		ModelInfo info{};

		std::string GetPart(const std::string& content, const std::string& target, const std::pair<char, char>& pair);
		std::vector<std::string> GetList(const std::string& content, const std::string& target, const std::pair<char, char>& pair);
		AttributeInfo GetAttribute(const std::string& accessContent, const std::string& viewContent) const;
		std::string GetValue(const std::string& content, const std::string& target)  const;
		void GetObjInfo(const std::string& name, size_t meshID);
		void GetGltfInfo(const std::string& name, size_t meshID);

	public:
		ModelLoader(const std::string& name, const ModelType& type, size_t meshID = 0);
		~ModelLoader();

		const ModelInfo& GetInfo() const;
		void GetBytes(char* address, const AttributeType& type);
};

class ImageLoader
{
	private:
		ImageInfo info{};

		void GetJpgInfo(const std::string& name);

	public:
		ImageLoader(const std::string& name, const ImageType& type);
		~ImageLoader();


};