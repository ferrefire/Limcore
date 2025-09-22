#pragma once

#include "point.hpp"
#include "vertex.hpp"
#include "structures.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <map>
#include <array>
#include <string>
#include <iostream>

#define FAST_BITS 8
#define CST(a) static_cast<size_t>(a)
#define C8(a) static_cast<uint8_t>(a)
#define C16(a) static_cast<uint16_t>(a)
#define C32(a) static_cast<uint32_t>(a)

static const std::array<size_t, 64> zigzagTable = 
	{
		0, 1, 8, 16, 9, 2, 3, 10,
		17, 24, 32, 25, 18, 11, 4, 5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13, 6, 7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63
	};

static const float sqrt12 = 1.0 / sqrt(2);

enum class ModelType { None, Obj, Gltf };
enum class AttributeType { None, Position, Normal, Coordinate, Color, Index };
enum class ImageType { None, Jpg, Png };
enum class ImageMarker 
	{ 
		SOI = 0xD8,
		EOI = 0xD9,
		SOS = 0xDA,
		SOF0 = 0xC0,
		APP0 = 0xE0,
		APP1 = 0xE1,
		APP2 = 0xE2,
		APP3 = 0xE3,
		APP4 = 0xE4,
		APP5 = 0xE5,
		APP6 = 0xE6,
		APP7 = 0xE7,
		APP8 = 0xE8,
		APP9 = 0xE9,
		APP10 = 0xEA,
		APP11 = 0xEB,
		APP12 = 0xEC,
		APP13 = 0xED,
		APP14 = 0xEE,
		APP15 = 0xEF,
		COM = 0xFE,
		DQT = 0xDB,
		DHT = 0xC4,
		DRI = 0xDD,
	};

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

struct SOF0Info
{
	size_t start = 0;
	size_t length = 0;
	size_t precision = 0;
	size_t height = 0;
	size_t width = 0;
	size_t componentCount = 0;
	std::vector<Point<size_t, 4>> components;
};

struct DQTInfo
{
	size_t start = 0;
	size_t length = 0;
	size_t precision = 0;
	size_t ID = 0;
	std::array<uint16_t, 64> values{};
};

struct HuffmanCode
{
	uint8_t symbol;
	uint8_t length;
	uint16_t code;
};

struct DHTInfo
{
	size_t start = 0;
	size_t length = 0;
	size_t type = 0;
	size_t ID = 0;

	std::vector<HuffmanCode> huffmanCodes;
};

struct SOSInfo
{
	size_t start = 0;
	size_t length = 0;
	size_t componentCount = 0;
	std::map<size_t, std::pair<size_t, size_t>> componentTables{};
};

typedef std::pair<bool, HuffmanCode> HuffmanResult;
typedef BinaryTree<HuffmanResult> HuffmanTree;
typedef std::array<int16_t, 64> DataBlock;

struct HuffmanTreeInfo
{
	HuffmanTree& root;
	std::map<std::string, uint8_t> mappedCodes;
};

struct ImageInfo
{
	std::string name = "";
	ImageType type = ImageType::None;

	SOF0Info startOfFrameInfo{};
	std::vector<DQTInfo> quantizationTables;
	std::vector<DHTInfo> huffmanInfos;
	SOSInfo startOfScanInfo{};
};

struct ImageData
{
	Point<size_t, 3> MCUCount{};

	std::vector<HuffmanTree> huffmanTables;
	std::vector<std::array<int16_t, 1 << FAST_BITS>> fastHuffmanTables;

	std::vector<DataBlock> blocks;
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

		bool AtMarker(ImageMarker marker);
		ImageMarker NextMarker();
};

class EntropyReader
{
	private:
		ByteReader& br;
		bool bitValue = false;
		uint16_t bitBuffer = 0;
		size_t index = 16;
		uint8_t previous = 0;

		int Extend(int v, int n);
		void AddBitsBuffer();
		void ReadBitBuffer();

	public:
		EntropyReader(ByteReader& br);

		HuffmanResult FindCode(const std::string& code, HuffmanTree& root) const;
		uint8_t NextSymbolFast(const HuffmanTree& tree);
		std::pair<bool, uint8_t> NextSymbolFast(const std::array<int16_t, 1 << FAST_BITS>& table, const std::vector<HuffmanCode>& codes);
		int ReadBitsBuffer(size_t amount);
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
		ImageData data{};

		void GetJpgInfo(const std::string& name);
		void BuildHuffmanTree(std::string current, HuffmanTree& start, std::vector<HuffmanCode>& codes);
		std::array<int16_t, 1 << FAST_BITS> BuildFastHuffmanTable(std::vector<HuffmanCode>& codes);
		uint8_t NextEntropySymbol(EntropyReader& er, size_t tableIndex);
		DataBlock IDCTBlock(const DataBlock& input);
		DataBlock FIDCTBlock(const DataBlock& input);

	public:
		ImageLoader(const std::string& name, const ImageType& type);
		~ImageLoader();

		const ImageInfo& GetInfo() const;

		void LoadEntropyData();
		void LoadBlock(std::array<unsigned char, (16 * 16) * 4>& buffer, size_t offset) const;
		void LoadPixels(std::vector<unsigned char>& buffer) const;
};

std::ostream& operator<<(std::ostream& out, const ImageInfo& info);