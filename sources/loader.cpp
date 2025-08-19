#include "loader.hpp"

#include "utilities.hpp"
#include "bitmask.hpp"
#include "printer.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

ByteReader::ByteReader(const uint8_t* data, size_t size) : position(data), end(data + size) {}

size_t ByteReader::BytesLeft() const
{
	return (CST(end - position));
}

size_t ByteReader::Offset(const uint8_t* base) const
{
	return (CST(position - base));
}

uint8_t ByteReader::Read8()
{
	if (position >= end) throw (std::runtime_error("Byte reader out of bounds (8)"));

	//return (*position++);

	uint8_t result = position[0];
	position++;
	
	return (result);
}

uint16_t ByteReader::Read16()
{
	if (end - position < 2) throw (std::runtime_error("Byte reader out of bounds (16)"));

	uint16_t result = ((C16(position[0]) << 8) | C16(position[1]));
	position += 2;

	return (result);
}

uint32_t ByteReader::Read32()
{
	if (end - position < 4) throw (std::runtime_error("Byte reader out of bounds (32)"));

	uint32_t result = ((C32(position[0]) << 24) | (C32(position[1]) << 16) | (C32(position[2]) << 8) | C32(position[3]));
	position += 4;

	return (result);
}

void ByteReader::Skip(size_t bytes)
{
	if (BytesLeft() < bytes) throw (std::runtime_error("Byte reader out of bounds (Skip)"));

	position += bytes;
}

bool ByteReader::AtMarker(ImageMarker marker)
{
	if (Read8() != 0xff) return (false);

	uint8_t byte = Read8();

	while (byte == 0xff) { byte = Read8(); }
	
	if (byte != C8(marker)) return (false);
	
	return (true);
}

ImageMarker ByteReader::NextMarker()
{
	while (Read8() != 0xff) { continue; }

	uint8_t byte = Read8();

	while (byte == 0xff) { byte = Read8(); }

	return (static_cast<ImageMarker>(byte));
}

point3D AttributeInfo::Translation() const
{
	point3D result;

	size_t start = 0;
	size_t end = translation.find(',');
	result.x() = std::stof(translation.substr(start, end - start));
	start = end + 1;
	end = translation.find(',');
	result.y() = std::stof(translation.substr(start, end - start));
	start = translation.rfind(',') + 1;
	result.z() = std::stof(translation.substr(start));

	return (result);
}

ModelLoader::ModelLoader(const std::string& name, const ModelType& type, size_t meshID)
{
	switch (type)
	{
		case ModelType::Obj: GetObjInfo(name, meshID); break;
		case ModelType::Gltf: GetGltfInfo(name, meshID); break;
		default: throw (std::runtime_error("Not a valid model type"));
	}
}

ModelLoader::~ModelLoader()
{

}

std::string ModelLoader::GetValue(const std::string& content, const std::string& target) const
{
	if (!content.contains(target)) return ("");

	size_t start = content.find(target) + target.size() + 3;
	size_t end = content.find('\n', start);

	return (content.substr(start, end - start));
}

AttributeInfo ModelLoader::GetAttribute(const std::string& accessContent, const std::string& viewContent) const
{
	AttributeInfo attributeInfo{};
	attributeInfo.accessContent = accessContent;
	attributeInfo.viewIndex = GetValue(attributeInfo.accessContent, "bufferView");
	attributeInfo.component = GetValue(attributeInfo.accessContent, "componentType");
	attributeInfo.count = GetValue(attributeInfo.accessContent, "count");
	attributeInfo.type = GetValue(attributeInfo.accessContent, "type");
	attributeInfo.viewContent = viewContent;
	attributeInfo.bufferIndex = GetValue(attributeInfo.viewContent, "buffer");
	attributeInfo.length = GetValue(attributeInfo.viewContent, "byteLength");
	attributeInfo.offset = GetValue(attributeInfo.viewContent, "byteOffset");

	return (attributeInfo);
}

std::string ModelLoader::GetPart(const std::string& content, const std::string& target, const std::pair<char, char>& pair)
{
	std::string result = "";

	std::pair<size_t, size_t> range = Utilities::FindPair(content, content.find(target), pair);
	if (range.first != range.second) result = content.substr(range.first + 1, range.second - range.first - 1);

	return (result);
}

std::vector<std::string> ModelLoader::GetList(const std::string& content, const std::string& target, const std::pair<char, char>& pair)
{
	std::vector<std::string> result;

	size_t end = 0;

	while (end != std::string::npos && end < content.size())
	{
		size_t start = end;
		if (target != "") start = content.find(target, end);
		std::pair<size_t, size_t> range = Utilities::FindPair(content, start, pair);
		if (range.first == range.second) break;
		if (range.first == std::string::npos || range.second == std::string::npos) break;
		if (range.first >= content.size() || range.second >= content.size()) break;
		result.push_back(content.substr(range.first + 1, range.second - range.first - 1));
		end = range.second + 1;
	}

	return (result);
}

void ModelLoader::GetObjInfo(const std::string& name, size_t meshID)
{
	info.name = name;
}

void ModelLoader::GetGltfInfo(const std::string& name, size_t meshID)
{
	info.name = name;
	info.type = ModelType::Gltf;

	std::string path = Utilities::GetPath() + "/resources/models/" + name + ".gltf";
	std::string file = Utilities::FileToString(path);

	std::string meshesInfo = GetPart(file, "meshes", {'[', ']'});

	std::vector<std::string> primitives = GetList(meshesInfo, "primitives", {'[', ']'});
	std::vector<std::string> meshes;

	for (std::string primitive : primitives)
	{
		std::vector<std::string> temp = GetList(primitive, "", {'{', '}'});
		meshes.insert(meshes.end(), temp.begin(), temp.end());
	}

	if (meshID >= meshes.size()) throw (std::runtime_error("Invalid mesh ID"));

	info.ID = meshID;
	info.count = meshes.size() - 1 - meshID;

	std::string positionsIndex = GetValue(meshes[meshID], "POSITION");
	std::string normalsIndex = GetValue(meshes[meshID], "NORMAL");
	std::string coordinatesIndex = GetValue(meshes[meshID], "TEXCOORD_0");
	std::string indicesIndex = GetValue(meshes[meshID], "indices");

	std::string accessInfo = GetPart(file, "accessors", {'[', ']'});
	std::vector<std::string> accessors = GetList(accessInfo, "", {'{', '}'});

	std::string viewInfo = GetPart(file, "bufferViews", {'[', ']'});
	std::vector<std::string> views = GetList(viewInfo, "", {'{', '}'});

	if (positionsIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Position);
		std::string accessContent = accessors[std::stoi(positionsIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Position] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Position].count));

		std::string nodeInfo = GetPart(file.substr(file.rfind("nodes")), "nodes", {'[', ']'});
		if (nodeInfo != "")
		{
			std::vector<std::string> nodes = GetList(nodeInfo, "", {'{', '}'});
			if (nodes.size() > meshID)
			{
				std::string translation = GetPart(nodes[meshID], "translation", {'[', ']'});
				if (translation != "") info.attributes[AttributeType::Position].translation = translation;
			}
		}
	}

	if (normalsIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Normal);
		std::string accessContent = accessors[std::stoi(normalsIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Normal] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Normal].count));
	}

	if (coordinatesIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Coordinate);
		std::string accessContent = accessors[std::stoi(coordinatesIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Coordinate] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Coordinate].count));
	}

	if (indicesIndex != "")
	{
		info.indexConfig = VK_INDEX_TYPE_UINT16;
		std::string accessContent = accessors[std::stoi(indicesIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[AttributeType::Index] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[AttributeType::Index].count));
	}
}

const ModelInfo& ModelLoader::GetInfo() const
{
	return (info);
}

const AttributeInfo& ModelInfo::GetAttribute(const AttributeType& type) const
{
	return (attributes.at(type));
}

void ModelLoader::GetBytes(char* address, const AttributeType& type)
{
	if (!info.attributes.contains(type)) throw (std::runtime_error("Model does not contain attribute type"));

	std::string path = Utilities::GetPath() + "/resources/models/" + info.name + ".bin";
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open()) throw (std::runtime_error("Failed to open file: " + info.name));

	file.seekg(info.attributes[type].Offset());
	file.read(address, info.attributes[type].Length());

	file.close();
}

ImageLoader::ImageLoader(const std::string& name, const ImageType& type)
{
	switch (type)
	{
		case ImageType::Jpg: GetJpgInfo(name); break;
		case ImageType::Png: return; break;
		default: throw (std::runtime_error("Not a valid image type"));
	}
}

ImageLoader::~ImageLoader()
{

}

void ImageLoader::GetJpgInfo(const std::string& name)
{
	info.name = name;
	info.type = ImageType::Jpg;

	std::string path = Utilities::GetPath() + "/resources/textures/" + name + ".jpg";
	std::string file = Utilities::FileToString(path);
	const uint8_t* data = reinterpret_cast<const uint8_t*>(file.c_str());

	ByteReader br(data, file.size());

	std::cout << "Total size: " << br.BytesLeft() << std::endl;
	std::cout << std::endl;

	if (br.AtMarker(ImageMarker::SOI)) std::cout << "SOI: " << br.BytesLeft() << std::endl << std::endl;

	while (br.BytesLeft() > 1)
	{
		ImageMarker next = br.NextMarker();

		if (next == ImageMarker::SOS)
		{
			std::cout << "SOS: " << br.BytesLeft() << std::endl;

			size_t length = CST(br.Read16()) - 2;
			std::cout << "Skipping: " << length << std::endl << std::endl;
			br.Skip(length);
		}
		else if (next == ImageMarker::SOF0)
		{
			std::cout << "SOF0: " << br.BytesLeft() << std::endl;

			info.startOfFrameInfo.length = CST(br.Read16());
			info.startOfFrameInfo.precision = CST(br.Read8());
			info.startOfFrameInfo.height = CST(br.Read16());
			info.startOfFrameInfo.width = CST(br.Read16());
			info.startOfFrameInfo.componentCount = CST(br.Read8());
			info.startOfFrameInfo.components.resize(info.startOfFrameInfo.componentCount);
			for (point4D& component : info.startOfFrameInfo.components)
			{
				component.x() = CST(br.Read8());
				uint8_t HV = br.Read8();
				component.y() = HV >> 4;
				component.z() = HV & 0x0F;
				component.w() = CST(br.Read8());
			}

			std::cout << "Length: " << info.startOfFrameInfo.length << std::endl;
			std::cout << "Precision: " << info.startOfFrameInfo.precision << std::endl;
			std::cout << "Height: " << info.startOfFrameInfo.height << std::endl;
			std::cout << "Width: " << info.startOfFrameInfo.width << std::endl;
			std::cout << "Component count: " << info.startOfFrameInfo.componentCount << std::endl;
			for (const point4D& component : info.startOfFrameInfo.components)
			{
				std::cout << component << std::endl;
			}
			std::cout << std::endl;
		}
		else if (next == ImageMarker::EOI)
		{
			std::cout << "EOI: " << br.BytesLeft() << std::endl;
			std::cout << std::endl;
		}
		else if ((next >= ImageMarker::APP0 && next <= ImageMarker::APP15) || next == ImageMarker::COM)
		{
			size_t length = CST(br.Read16()) - 2;
			std::cout << "Skipping: " << length << std::endl << std::endl;
			br.Skip(length);
		}
		else if (next == ImageMarker::DQT)
		{
			std::cout << "DQT: " << br.BytesLeft() << std::endl;

			DQTInfo quantizationTable{};
			quantizationTable.length = CST(br.Read16());
			uint8_t PT = br.Read8();
			quantizationTable.precision = CST(PT >> 4);
			quantizationTable.ID = CST(PT & 0x0F);

			for (size_t i = 0; i < 64; i++)
			{
				quantizationTable.values[zigzagTable[i]] = br.Read8();
			}

			info.quantizationTables.push_back(quantizationTable);

			std::cout << "Length: " << quantizationTable.length << std::endl;
			std::cout << "Precision: " << quantizationTable.precision << std::endl;
			std::cout << "ID: " << quantizationTable.ID << std::endl;
			for (size_t r = 0; r < 8; r++)
			{
				for (size_t c = 0; c < 8; c++)
				{
					std::cout << CST(quantizationTable.values[r * 8 + c]);
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		else if (next == ImageMarker::DHT)
		{
			std::cout << "DHT: " << br.BytesLeft() << std::endl;

			size_t length = CST(br.Read16()) - 2;
			std::cout << "Skipping: " << length << std::endl << std::endl;
			br.Skip(length);
		}
		else if (next == ImageMarker::DRI)
		{
			std::cout << "DRI: " << br.BytesLeft() << std::endl;

			size_t length = CST(br.Read16()) - 2;
			std::cout << "Skipping: " << length << std::endl << std::endl;
			br.Skip(length);
		}
	}
}