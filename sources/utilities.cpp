#include "utilities.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <iostream>

std::string Utilities::GetPath()
{
	std::filesystem::path currentPath = std::filesystem::current_path();

	while (currentPath.string().contains("build"))
	{
		currentPath = currentPath.parent_path();
	}

	return (currentPath.string());
}

std::vector<char> Utilities::FileToBinary(const std::string& path)
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

std::string Utilities::FileToString(const std::string& path)
{
	std::string result;
	std::ifstream file(path.c_str());

	if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);

	std::stringstream stream;
	stream << file.rdbuf();

	file.close();
	result = stream.str();
	
	return (result);
}

std::vector<std::string> Utilities::Split(const std::string& string, const std::string& delimitter)
{
	std::vector<std::string> result;
	size_t start = 0;
	size_t end = string.find(delimitter);

	while (end != std::string::npos)
	{
		if (end - start > 0) result.push_back(string.substr(start, (end - start)));

		start = end + 1;
		end = string.find(delimitter, start);
	}

	if (start != std::string::npos) result.push_back(string.substr(start));
	
	return (result);
}

std::vector<std::string> Utilities::FileToLines(const std::string& path)
{
	std::vector<std::string> result;
	std::string file = Utilities::FileToString(path);

	result = Utilities::Split(file, "\n");
	
	return (result);
}

/*std::pair<size_t, size_t> Utilities::FindPair(const std::string& string, size_t begin, const std::pair<char, char>& pair)
{
	std::pair<size_t, size_t> result(begin, begin);

	size_t start = string.find(pair.first, begin);

	if (start != std::string::npos) result.first = start;
	else return (result);

	size_t current = start + 1;
	size_t next = string.find(pair.first, current);
	size_t end = string.find(pair.second, current);

	while (next != std::string::npos && next < end)
	{
		current = end + 1;
		next = string.find(pair.first, current);
		end = string.find(pair.second, current);
	}

	if (end != std::string::npos) result.second = end;

	return (result);
}*/

std::pair<size_t, size_t> Utilities::FindPair(const std::string& string, size_t begin, const std::pair<char, char>& pair)
{
	std::pair<size_t, size_t> result(begin, begin);

	size_t start = string.find(pair.first, begin);
	size_t position = start;

	size_t openings = 1;
	size_t closings = 0;

	while (openings != closings)
	{
		if (position == std::string::npos || position >= string.size()) break;

		position++;

		size_t nextOpening = string.find(pair.first, position);
		size_t nextClosing = string.find(pair.second, position);

		if (nextOpening < nextClosing) openings++;
		else if (nextClosing < nextOpening) closings++;

		position = std::min(nextOpening, nextClosing);
	}
	
	if (openings != closings) return (result);

	result.first = start;
	result.second = position;

	return (result);
}

float Utilities::Radians(float degrees)
{
	return (degrees * 0.0174532925);
}