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
	
	if constexpr (hasPosition)
	{
		vertices[0].position = point3D(-0.5f, -0.5f, 0.0f);
		vertices[1].position = point3D(0.5f, -0.5f, 0.0f);
		vertices[2].position = point3D(0.5f, 0.5f, 0.0f);
		vertices[3].position = point3D(-0.5f, 0.5f, 0.0f);
		
	}

	if constexpr (hasCoordinate)
	{
		vertices[0].coordinate = point2D(0.0f, 0.0f);
		vertices[1].coordinate = point2D(1.0f, 0.0f);
		vertices[2].coordinate = point2D(1.0f, 1.0f);
		vertices[3].coordinate = point2D(0.0f, 1.0f);
	}

	if constexpr (hasNormal)
	{
		vertices[0].normal = point3D(0, 0, -1);
		vertices[1].normal = point3D(0, 0, -1);
		vertices[2].normal = point3D(0, 0, -1);
		vertices[3].normal = point3D(0, 0, -1);
	}

	if constexpr (hasIndices)
	{
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(0);
		indices.push_back(2);
		indices.push_back(3);
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::CreateCube()
{
	Shape<V, I> near(ShapeType::Quad);
	near.Move(point3D(0, 0, -0.5));

	Shape<V, I> far(ShapeType::Quad);
	far.Rotate(180, Axis::y);
	far.Move(point3D(0, 0, 0.5));

	Shape<V, I> right(ShapeType::Quad);
	right.Rotate(90, Axis::y);
	right.Move(point3D(0.5, 0, 0));

	Shape<V, I> left(ShapeType::Quad);
	left.Rotate(-90, Axis::y);
	left.Move(point3D(-0.5, 0, 0));

	Shape<V, I> top(ShapeType::Quad);
	top.Rotate(90, Axis::x);
	top.Move(point3D(0, 0.5, 0));

	Shape<V, I> bottom(ShapeType::Quad);
	bottom.Rotate(-90, Axis::x);
	bottom.Move(point3D(0, -0.5, 0));

	Join(near);
	Join(far);
	Join(right);
	Join(left);
	Join(top);
	Join(bottom);
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

SHAPE_TEMPLATE
void Shape<V, I>::Move(const point3D& translation)
{
	if constexpr (hasPosition)
	{
		for (Vertex<V>& vertex : vertices)
		{
			vertex.position += translation;
		}
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Rotate(const float& degrees, const Axis& axis)
{
	for (Vertex<V>& vertex : vertices)
	{
		if constexpr (hasPosition)
		{
			vertex.position.Rotate(degrees, axis);
		}

		if constexpr (hasNormal)
		{
			vertex.normal.Rotate(degrees, axis);
			vertex.normal.Unitize();
		}
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Join(const Shape<V, I>& other)
{
	const indexType count = static_cast<indexType>(vertices.size());

	for (const Vertex<V>& vertex : other.GetVertices())
	{
		vertices.push_back(vertex);
	}

	if (hasIndices)
	{
		for (const indexType& index : other.GetIndices())
		{
			indices.push_back(index + count);
		}
	}
}