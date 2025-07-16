#include "shape.hpp"

SHAPE_TEMPLATE
Shape<V, I>::Shape()
{

}

SHAPE_TEMPLATE
Shape<V, I>::Shape(ShapeType type)
{
	Create(type);
}

SHAPE_TEMPLATE
Shape<V, I>::~Shape()
{
	
}

SHAPE_TEMPLATE
void Shape<V, I>::Create(ShapeType type)
{
	switch (type)
	{
		case ShapeType::Quad: CreateQuad(); break;
		case ShapeType::Cube: CreateCube(); break;
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::CreateQuad()
{
	vertices.resize(4);
	
	if (hasPosition)
	{
		vertices[0].position = point3D({-0.5f, -0.5f, 0.0f});
		vertices[1].position = point3D({0.5f, 0.5f, 0.0f});
		vertices[2].position = point3D({-0.5f, 0.5f, 0.0f});
		vertices[3].position = point3D({0.5f, -0.5f, 0.0f});
	}

	if (hasCoordinate)
	{
		vertices[0].coordinate = point2D({0.0f, 0.0f});
		vertices[1].coordinate = point2D({1.0f, 1.0f});
		vertices[2].coordinate = point2D({0.0f, 1.0f});
		vertices[3].coordinate = point2D({1.0f, 0.0f});
	}

	if (hasIndices)
	{
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(1);
		indices.push_back(0);
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::CreateCube()
{

}

SHAPE_TEMPLATE
const std::vector<Vertex<V>>& Shape<V, I>::GetVertices() const
{
	return (vertices);
}

SHAPE_TEMPLATE
const std::vector<std::conditional_t<I == VK_INDEX_TYPE_UINT16, uint16_t, uint32_t>>& Shape<V, I>::GetIndices() const
{
	return (indices);
}