#include "vertex.hpp"

VERTEX_TEMPLATE
std::vector<float> Vertex<V>::GetData()
{
	size_t size = ((Bitmask::HasFlag(V, Position) ? 3 : 0) + (Bitmask::HasFlag(V, Coordinate) ? 2 : 0) +
		(Bitmask::HasFlag(V, Normal) ? 3 : 0) + (Bitmask::HasFlag(V, Color) ? 3 : 0));

	std::vector<float> data(size);

	uint32_t i = 0;

	if constexpr (Bitmask::HasFlag(V, Position))
	{
		data[i++] = this->position[0];
		data[i++] = this->position[1];
		data[i++] = this->position[2];
	}
	if constexpr (Bitmask::HasFlag(V, Coordinate))
	{
		data[i++] = this->coordinate[0];
		data[i++] = this->coordinate[1];
	}
	if constexpr (Bitmask::HasFlag(V, Normal))
	{
		data[i++] = this->normal[0];
		data[i++] = this->normal[1];
		data[i++] = this->normal[2];
	}
	if constexpr (Bitmask::HasFlag(V, Color))
	{
		data[i++] = this->color[0];
		data[i++] = this->color[1];
		data[i++] = this->color[2];
	}

	return (data);
}

VERTEX_TEMPLATE
std::ostream& operator<<(std::ostream& out, Vertex<V> vertex)
{
	if constexpr (Bitmask::HasFlag(V, Position)) out << "position: " << vertex.position << " ";
	if constexpr (Bitmask::HasFlag(V, Coordinate)) out << "coordinate: " << vertex.coordinate << " ";
	if constexpr (Bitmask::HasFlag(V, Normal)) out << "normal: " << vertex.normal << " ";
	if constexpr (Bitmask::HasFlag(V, Color)) out << "color: " << vertex.color << " ";

	return (out);
}