#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>

#define CI(a) static_cast<int>(a)
#define CUI(a) static_cast<uint32_t>(a)

class Utilities
{
	private:
		

	public:
		static std::string GetPath();
		static std::vector<char> FileToBinary(const std::string& path);
		static std::string FileToString(const std::string& path);
		static std::vector<std::string> Split(const std::string& string, const std::string& delimitter);
		static std::vector<std::string> FileToLines(const std::string& path);
		static std::pair<size_t, size_t> FindPair(const std::string& string, size_t start, const std::pair<char, char>& pair);
		
		template <typename T>
		requires (std::is_integral<T>().value)
		static std::string ToBits(T byte);
		
		template <typename T>
		static bool Contains(const std::vector<T>& vector, T target);

		static float Radians(float degrees);
};

#include "utilities.tpp"