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
Shape<V, I>::Shape(ModelInfo info)
{
	Create(info);
}

SHAPE_TEMPLATE
Shape<V, I>::~Shape()
{
	
}

SHAPE_TEMPLATE
void Shape<V, I>::Create(ShapeType type)
{
	Destroy();

	switch (type)
	{
		case ShapeType::Quad: CreateQuad(); break;
		case ShapeType::Cube: CreateCube(); break;
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Create(ModelInfo info)
{
	Destroy();

	vertices.resize(info.size);

	if constexpr (hasPosition)
	{
		if (Bitmask::HasFlag(info.vertexConfig, Position))
		{
			AttributeInfo& attributeInfo = info.attributes[Position];
			std::vector<point3D> positions(attributeInfo.Count());
			size_t offset = attributeInfo.Offset();
			size_t length = attributeInfo.Length();
			Loader::GetBytes(info.name, reinterpret_cast<char*>(positions.data()), offset, length);

			size_t i = 0;
			for (const point3D& point : positions) { vertices[i++].position = point; }
		}
	}

	if constexpr (hasNormal)
	{
		if (Bitmask::HasFlag(info.vertexConfig, Normal))
		{
			AttributeInfo& attributeInfo = info.attributes[Normal];
			std::vector<point3D> normals(attributeInfo.Count());
			size_t offset = attributeInfo.Offset();
			size_t length = attributeInfo.Length();
			Loader::GetBytes(info.name, reinterpret_cast<char*>(normals.data()), offset, length);

			size_t i = 0;
			for (const point3D& point : normals) { vertices[i++].normal = point; }
		}
	}

	if constexpr (hasIndices)
	{
		if (info.indexConfig != VK_INDEX_TYPE_NONE_KHR)
		{
			AttributeInfo& attributeInfo = info.indexInfo;
			indices.resize(attributeInfo.Count());
			size_t offset = attributeInfo.Offset();
			size_t length = attributeInfo.Length();
			Loader::GetBytes(info.name, reinterpret_cast<char*>(indices.data()), offset, length);
		}
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Destroy()
{
	vertices.clear();
	indices.clear();
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
		vertices[0].normal = point3D(0, 0, 1);
		vertices[1].normal = point3D(0, 0, 1);
		vertices[2].normal = point3D(0, 0, 1);
		vertices[3].normal = point3D(0, 0, 1);
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
	Shape<V, I> far(ShapeType::Quad);
	far.Move(point3D(0, 0, 0.5));

	Shape<V, I> near(ShapeType::Quad);
	near.Rotate(180, Axis::y);
	near.Move(point3D(0, 0, -0.5));

	Shape<V, I> right(ShapeType::Quad);
	right.Rotate(-90, Axis::y);
	right.Move(point3D(0.5, 0, 0));

	Shape<V, I> left(ShapeType::Quad);
	left.Rotate(90, Axis::y);
	left.Move(point3D(-0.5, 0, 0));

	Shape<V, I> top(ShapeType::Quad);
	top.Rotate(-90, Axis::x);
	top.Move(point3D(0, 0.5, 0));

	Shape<V, I> bottom(ShapeType::Quad);
	bottom.Rotate(90, Axis::x);
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
void Shape<V, I>::Centerize()
{
	if (!hasPosition) return;

	point3D boundsX;
	point3D boundsY;
	point3D boundsZ;

	for (const Vertex<V>& vertex : vertices)
	{
		if (vertex.position.x() < boundsX.x()) boundsX.x() = vertex.position.x();
		if (vertex.position.x() > boundsX.y()) boundsX.y() = vertex.position.x();

		if (vertex.position.y() < boundsY.x()) boundsY.x() = vertex.position.y();
		if (vertex.position.y() > boundsY.y()) boundsY.y() = vertex.position.y();

		if (vertex.position.z() < boundsZ.x()) boundsZ.x() = vertex.position.z();
		if (vertex.position.z() > boundsZ.y()) boundsZ.y() = vertex.position.z();
	}

	point3D center;
	center.x() = (boundsX.x() + boundsX.y()) / 2.0;
	center.y() = (boundsY.x() + boundsY.y()) / 2.0;
	center.z() = (boundsZ.x() + boundsZ.y()) / 2.0;

	for (Vertex<V>& vertex : vertices)
	{
		vertex.position -= center;
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Scalarize()
{
	if (!hasPosition) return;

	Centerize();

	float max = 0;

	for (const Vertex<V>& vertex : vertices)
	{
		max = std::max(std::abs(vertex.position.x()), max);
		max = std::max(std::abs(vertex.position.y()), max);
		max = std::max(std::abs(vertex.position.z()), max);
	}

	float divider = max * 2;

	for (Vertex<V>& vertex : vertices)
	{
		vertex.position /= divider;
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