#include "utilities.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

std::string Utilities::GetPath()
{
	std::filesystem::path currentPath = std::filesystem::current_path();

	while (currentPath.string().contains("build"))
	{
		currentPath = currentPath.parent_path();
	}

	return (currentPath.string());
}

std::vector<char> Utilities::FileToBinary(std::string path)
{
	std::ifstream file(path.c_str(), std::ios::ate | std::ios::binary);

	if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> result(fileSize);

	file.seekg(0);
	file.read(result.data(), fileSize);
	file.close();

	return (result);
}