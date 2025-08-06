#include "loader.hpp"

#include "utilities.hpp"
#include "bitmask.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

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
	std::string path = Utilities::GetPath() + "/resources/" + name + ".gltf";
	std::string file = Utilities::FileToString(path);

	size_t meshes = file.find("meshes");
	if (meshes == std::string::npos) throw (std::runtime_error("Gltf file is invalid"));

	std::pair<size_t, size_t> range = Utilities::FindPair(file, meshes, {'[', ']'});
	if (range.first == range.second) throw (std::runtime_error("Gltf file is invalid"));

	std::string meshesInfo = file.substr(range.first + 1, range.second - range.first - 1);

	size_t primitives = meshesInfo.find("primitives");
	if (primitives == std::string::npos) throw (std::runtime_error("Gltf file is invalid"));

	range = Utilities::FindPair(meshesInfo, primitives, {'[', ']'});
	if (range.first == range.second) throw (std::runtime_error("Gltf file is invalid"));

	std::string primitivesInfo = meshesInfo.substr(range.first + 1, range.second - range.first - 1);

	size_t attributes = primitivesInfo.find("attributes");
	if (attributes == std::string::npos) throw (std::runtime_error("Gltf file is invalid"));

	range = Utilities::FindPair(primitivesInfo, attributes, {'{', '}'});
	if (range.first == range.second) throw (std::runtime_error("Gltf file is invalid"));

	std::string attributesInfo = primitivesInfo.substr(range.first + 1, range.second - range.first - 1);

	//std::cout << attributesInfo << std::endl;

	if (attributesInfo.contains("POSITION"))
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Position);
		info.attributes[Position] = AttributeInfo{};
		info.attributes[Position].type = Position;
		info.attributes[Position].index = std::stoi(attributesInfo.substr(attributesInfo.find("POSITION") + 11, 1));
	}
	if (attributesInfo.contains("NORMAL"))
	{
		info.vertexConfig = Bitmask::SetFlag(info.vertexConfig, Normal);
		info.attributes[Normal] = AttributeInfo{};
		info.attributes[Normal].type = Normal;
		info.attributes[Normal].index = std::stoi(attributesInfo.substr(attributesInfo.find("NORMAL") + 9, 1));
	}

	if (primitivesInfo.contains("indices"))
	{
		info.indexInfo.index = std::stoi(primitivesInfo.substr(primitivesInfo.find("indices") + 10, 1));
	}

	//Continue by parsing accessor information!!!!!!1

	return (info);
}

ModelInfo Loader::GetObjInfo(const std::string& name)
{
	ModelInfo data{};
	//std::string path = Utilities::GetPath() + "/resources/" + name + ".gltf";
	//std::string file = Utilities::FileToString(path);

	

	return (data);
}

/*ModelData Loader::LoadModel(const std::string& name, const ModelType& type)
{
	switch (type)
	{
		case ModelType::Obj: return (LoadObj(name)); break;
		case ModelType::Gltf: return (LoadGltf(name)); break;
	}

	throw (std::runtime_error("Not a valid mode type"));
}

ModelData Loader::LoadObj(const std::string& name)
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
}

ModelData Loader::LoadGltf(const std::string& name)
{
	ModelData data{};
	std::string path = Utilities::GetPath();
	std::string fullName = path + "/resources/" + name + ".bin";

	std::ifstream file(fullName.c_str(), std::ios::binary);

	if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);

	file.seekg(0);
	data.positions.resize(1031);
	file.read(reinterpret_cast<char*>(data.positions.data()), 1031 * 3 * sizeof(float));

	file.seekg(12372);
	data.normals.resize(1031);
	file.read(reinterpret_cast<char*>(data.normals.data()), 1031 * 3 * sizeof(float));

	file.seekg(32992);
	data.indices.resize(5970);
	file.read(reinterpret_cast<char*>(data.indices.data()), 5970 * sizeof(uint16_t));
	
	file.close();

	data.size = data.positions.size();

	return (data);
}*/