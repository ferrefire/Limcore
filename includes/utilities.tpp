#include "utilities.hpp"

template <typename T>
bool Utilities::Contains(const std::vector<T>& vector, T target)
{
	for (const T& element : vector) { if (element == target) return (true); }

	return (false);
}