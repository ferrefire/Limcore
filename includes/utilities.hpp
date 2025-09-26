#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <type_traits>
#include <filesystem>

#define CI(a) static_cast<int>(a)
#define CUI(a) static_cast<uint32_t>(a)

/**
 * @file utilities.hpp
 * @brief General-purpose utility functions for file I/O, string manipulation, and math.
 *
 * @details
 * Provides helper macros and a static utility class containing methods for
 * reading files, splitting strings, searching, bit manipulation, and math conversions.
 */

/**
 * @brief Static utility class with common helper functions.
 *
 * @details
 * Contains file reading, string splitting, search helpers, math conversions,
 * and generic container utilities.
 */
class Utilities
{
	private:
		

	public:
		/**
		 * @brief Gets the current working path or application path.
		 * @return String representing the path.
		 */
		static std::string GetPath();

		/**
		 * @brief Reads a file into a binary buffer.
		 * @param path Path to the file.
		 * @return Vector of bytes containing file contents.
		 */
		static std::vector<char> FileToBinary(const std::string& path);

		/**
		 * @brief Reads a file into a string.
		 * @param path Path to the file.
		 * @return File contents as a single string.
		 */
		static std::string FileToString(const std::string& path);

		/**
		 * @brief Splits a string into tokens by a delimiter.
		 * @param string Input string.
		 * @param delimitter Delimiter string.
		 * @return Vector of tokens.
		 */
		static std::vector<std::string> Split(const std::string& string, const std::string& delimitter);

		/**
		 * @brief Reads a file and returns its lines.
		 * @param path Path to the file.
		 * @return Vector of strings, each representing one line.
		 */
		static std::vector<std::string> FileToLines(const std::string& path);

		/**
		 * @brief Finds the matching pair of characters starting from a position.
		 * @param string Input string.
		 * @param start Starting position.
		 * @param pair Pair of characters (open, close).
		 * @return Index pair (open position, close position).
		 */
		static std::pair<size_t, size_t> FindPair(const std::string& string, size_t start, const std::pair<char, char>& pair);
		
		/**
		 * @brief Converts a byte/integral value to a string of bits.
		 * @tparam T Integral type.
		 * @param byte Value to convert.
		 * @return String of bits (e.g., "01010101").
		 */
		template <typename T>
		requires (std::is_integral<T>().value)
		static std::string ToBits(T byte);
		
		/**
		 * @brief Checks if a vector contains a target value.
		 * @tparam T Element type.
		 * @param vector Container to search.
		 * @param target Value to find.
		 * @return True if found, false otherwise.
		 */
		template <typename T>
		static bool Contains(const std::vector<T>& vector, T target);

		/**
		 * @brief Converts degrees to radians.
		 * @param degrees Angle in degrees.
		 * @return Angle in radians.
		 */
		static float Radians(float degrees);

		static bool HasDirectory(const std::filesystem::path& path, const std::string& directory);
};

#include "utilities.tpp"