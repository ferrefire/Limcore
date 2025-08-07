#include "loader.hpp"

#include "utilities.hpp"
#include "bitmask.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

std::string Loader::GetValue(const std::string& content, const std::string& target)
{
	if (!content.contains(target)) return ("");

	size_t start = content.find(target) + target.size() + 3;
	size_t end = content.find('\n', start);

	return (content.substr(start, end - start));
}

AttributeInfo Loader::GetAttribute(const std::string& accessContent, const std::string& viewContent)
{
	AttributeInfo info{};
	info.accessContent = accessContent;
	info.viewIndex = GetValue(info.accessContent, "bufferView");
	info.component = GetValue(info.accessContent, "componentType");
	info.count = GetValue(info.accessContent, "count");
	info.type = GetValue(info.accessContent, "type");
	info.viewContent = viewContent;
	info.bufferIndex = GetValue(info.viewContent, "buffer");
	info.length = GetValue(info.viewContent, "byteLength");
	info.offset = GetValue(info.viewContent, "byteOffset");

	return (info);
}

ModelInfo Loader::GetModelInfo(const std::string& name, const ModelType& type)
{
	switch (type)
	{
		case ModelType::Obj: return (GetObjInfo(name)); break;
		case ModelType::Gltf: return (GetGltfInfo(name)); break;
	}

	throw (std::runtime_error("Not a valid model type"));
}

ModelInfo Loader::GetGltfInfo(const std::string& name)
{
	ModelInfo info{};

	info.name = name;

	std::string path = Utilities::GetPath() + "/resources/" + name + ".gltf";
	std::string file = Utilities::FileToString(path);

	std::pair<size_t, size_t> range = Utilities::FindPair(file, file.find("meshes"), {'[', ']'});
	if (range.first == range.second) throw (std::runtime_error("Gltf file is invalid"));
	std::string meshesInfo = file.substr(range.first + 1, range.second - range.first - 1);

	std::string positionsIndex = GetValue(meshesInfo, "POSITION");
	std::string normalsIndex = GetValue(meshesInfo, "NORMAL");
	std::string indicesIndex = GetValue(meshesInfo, "indices");

	range = Utilities::FindPair(file, file.find("accessors"), {'[', ']'});
	if (range.first == range.second) throw (std::runtime_error("Gltf file is invalid"));
	std::string accessInfo = file.substr(range.first + 1, range.second - range.first - 1);
	std::vector<std::string> accessors;
	size_t end = 0;

	while (end != std::string::npos && end < accessInfo.size())
	{
		range = Utilities::FindPair(accessInfo, end, {'{', '}'});
		accessors.push_back(accessInfo.substr(range.first + 1, range.second - range.first - 1));
		end = range.second + 1;
	}

	range = Utilities::FindPair(file, file.find("bufferViews"), {'[', ']'});
	if (range.first == range.second) throw (std::runtime_error("Gltf file is invalid"));
	std::string viewInfo = file.substr(range.first + 1, range.second - range.first - 1);
	std::vector<std::string> views;
	end = 0;

	while (end != std::string::npos && end < viewInfo.size())
	{
		range = Utilities::FindPair(viewInfo, end, {'{', '}'});
		views.push_back(viewInfo.substr(range.first + 1, range.second - range.first - 1));
		end = range.second + 1;
	}

	if (positionsIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Position);
		std::string accessContent = accessors[std::stoi(positionsIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[Position] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[Position].count));
	}

	if (normalsIndex != "")
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Normal);
		std::string accessContent = accessors[std::stoi(normalsIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.attributes[Normal] = GetAttribute(accessContent, viewContent);
		info.size = std::max(info.size, std::stoul(info.attributes[Normal].count));
	}

	if (indicesIndex != "")
	{
		info.indexConfig = VK_INDEX_TYPE_UINT16;
		std::string accessContent = accessors[std::stoi(indicesIndex)];
		std::string viewContent = views[std::stoi(GetValue(accessContent, "bufferView"))];
		info.indexInfo = GetAttribute(accessContent, viewContent);
	}

	return (info);
}

ModelInfo Loader::GetObjInfo(const std::string& name)
{
	ModelInfo data{};
	//std::string path = Utilities::GetPath() + "/resources/" + name + ".gltf";
	//std::string file = Utilities::FileToString(path);

	

	return (data);
}

void Loader::GetBytes(const std::string& name, char* address, size_t offset, size_t size)
{
	std::string path = Utilities::GetPath() + "/resources/" + name + ".bin";
	std::ifstream file(path, std::ios::binary);

	if (!file.is_open()) throw (std::runtime_error("Failed to open file: " + name));

	file.seekg(offset);
	file.read(address, size);

	file.close();
}

/*ModelData Loader::LoadObj(const std::string& name)
{
	ModelData data{};
	std::string path = Utilities::GetPath();
	std::string fullName = path + "/resources/" + name + ".obj";

	std::vector<std::string> lines = Utilities::FileToLines(fullName);

	for (const std::string& line : lines)
	{
		if (line.find("v ") == 0)
		{
			point3D point = point3D::FromString(line.substr(2));
			data.positions.push_back(point);
		}
		else if (line.find("f ") == 0)
		{
			std::vector<std::string> values = Utilities::Split(line.substr(2), " ");

			if (values.size() == 3)
			{
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[0]) - 1));
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[1]) - 1));
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[2]) - 1));
			}
			else if (values.size() == 4)
			{
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[0]) - 1));
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[1]) - 1));
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[2]) - 1));

				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[0]) - 1));
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[2]) - 1));
				data.indices.push_back(static_cast<uint16_t>(std::stoi(values[3]) - 1));
			}
		}
	}

	data.size = data.positions.size();

	return (data);
}*/