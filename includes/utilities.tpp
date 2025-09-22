#include "utilities.hpp"

#include <bitset>

template <typename T>
bool Utilities::Contains(const std::vector<T>& vector, T target)
{
	for (const T& element : vector) { if (element == target) return (true); }

	return (false);
}

template <typename T>
requires (std::is_integral<T>().value)
std::string Utilities::ToBits(T byte)
{
	std::bitset<sizeof(T) * 8> bs(byte);
	
	return (bs.to_string());
}