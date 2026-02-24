#include "shape.hpp"

SHAPE_TEMPLATE
Shape<V, I>::Shape()
{

}

SHAPE_TEMPLATE
Shape<V, I>::Shape(ShapeType type, ShapeSettings shapeSettings)
{
	Create(type, shapeSettings);
}

SHAPE_TEMPLATE
Shape<V, I>::Shape(ModelLoader loader, ShapeSettings shapeSettings)
{
	Create(loader, shapeSettings);
}

SHAPE_TEMPLATE
Shape<V, I>::~Shape()
{
	
}

SHAPE_TEMPLATE
void Shape<V, I>::Create(ShapeType type, ShapeSettings shapeSettings)
{
	Destroy();

	settings = shapeSettings;

	switch (type)
	{
		case ShapeType::Quad: CreateQuad(); break;
		case ShapeType::Cube: CreateCube(); break;
		case ShapeType::Plane: CreatePlane(); break;
		case ShapeType::Cylinder: CreateCylinder(); break;
	}

	if (settings.scalarized) Scalarize();
}

SHAPE_TEMPLATE
void Shape<V, I>::Create(ModelLoader loader, ShapeSettings shapeSettings)
{
	Destroy();

	settings = shapeSettings;

	const ModelInfo& info = loader.GetInfo();
	vertices.resize(info.size);

	if constexpr (hasPosition)
	{
		if (Bitmask::HasFlag(info.vertexConfig, Position))
		{
			std::vector<point3D> positions(info.GetAttribute(AttributeType::Position).Count());
			loader.GetBytes(reinterpret_cast<char*>(positions.data()), AttributeType::Position);

			point3D offset = info.GetAttribute(AttributeType::Position).Translation();

			size_t i = 0;
			for (const point3D& point : positions) { vertices[i++].position = point + offset; }
		}
	}

	if constexpr (hasNormal)
	{
		if (Bitmask::HasFlag(info.vertexConfig, Normal))
		{
			std::vector<point3D> normals(info.GetAttribute(AttributeType::Normal).Count());
			loader.GetBytes(reinterpret_cast<char*>(normals.data()), AttributeType::Normal);

			size_t i = 0;
			for (const point3D& point : normals) { vertices[i++].normal = point; }
		}
	}

	if constexpr (hasCoordinate)
	{
		if (Bitmask::HasFlag(info.vertexConfig, Coordinate))
		{
			std::vector<point2D> coordinates(info.GetAttribute(AttributeType::Coordinate).Count());
			loader.GetBytes(reinterpret_cast<char*>(coordinates.data()), AttributeType::Coordinate);

			size_t i = 0;
			for (const point2D& point : coordinates) { vertices[i++].coordinate = point; }
		}
	}

	if constexpr (hasIndices)
	{
		if (info.indexConfig != VK_INDEX_TYPE_NONE_KHR)
		{
			indices.resize(info.GetAttribute(AttributeType::Index).Count());
			std::vector<uint16_t> tempIndices(info.GetAttribute(AttributeType::Index).Count());
			loader.GetBytes(reinterpret_cast<char*>(tempIndices.data()), AttributeType::Index);

			size_t i = 0;
			for (const uint16_t& index : tempIndices) { indices[i++] = static_cast<indexType>(index); }
		}
	}

	if (info.count > 0)
	{
		Shape<V, I> other(ModelLoader(info.name, info.type, info.ID + 1), {false, 1});
		Join(other);
	}

	if (settings.scalarized) Scalarize();
}

SHAPE_TEMPLATE
void Shape<V, I>::Destroy()
{
	vertices.clear();
	indices.clear();
	settings = ShapeSettings{};
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

	if constexpr (hasNormal)
	{
		vertices[0].normal = point3D(0, 0, 1);
		vertices[1].normal = point3D(0, 0, 1);
		vertices[2].normal = point3D(0, 0, 1);
		vertices[3].normal = point3D(0, 0, 1);
	}

	if constexpr (hasCoordinate)
	{
		vertices[0].coordinate = point2D(0.0f, 0.0f);
		vertices[1].coordinate = point2D(1.0f, 0.0f);
		vertices[2].coordinate = point2D(1.0f, 1.0f);
		vertices[3].coordinate = point2D(0.0f, 1.0f);
	}

	if constexpr (hasColor)
	{
		vertices[0].color = point3D(1.0f);
		vertices[1].color = point3D(1.0f);
		vertices[2].color = point3D(1.0f);
		vertices[3].color = point3D(1.0f);
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
void Shape<V, I>::CreatePlane()
{
	for (size_t i = 0, x = 0; x <= settings.resolution.x(); ++x)
    {
        for (size_t z = 0; z <= settings.resolution.y(); ++z, ++i)
        {
			vertices.push_back(Vertex<V>{});

			float xPos = ((float)x / settings.resolution.x()) - 0.5f;
			float zPos = ((float)z / settings.resolution.y()) - 0.5f;

			if constexpr (hasPosition)
			{
				vertices[vertices.size() - 1].position = point3D(xPos, 0, zPos);
			}

			if constexpr (hasNormal)
			{
				vertices[vertices.size() - 1].normal = point3D(0, 1, 0);
			}

			if constexpr (hasCoordinate)
			{
				vertices[vertices.size() - 1].coordinate = point2D(xPos + 0.5f, zPos + 0.5f);
			}

			if constexpr (hasColor)
			{
				vertices[vertices.size() - 1].color = point3D(1.0f);
			}
        }
	}

	if constexpr (hasIndices)
	{
		for (size_t ti = 0, vi = 0, x = 0; x < settings.resolution.x(); ++vi, ++x)
    	{
    	    for (size_t z = 0; z < settings.resolution.y(); ti += 6, ++vi, ++z)
    	    {
    	        indices.push_back(vi);
    	        indices.push_back(vi + 1);
    	        indices.push_back(vi + settings.resolution.y() + 2);
    	        indices.push_back(vi);
    	        indices.push_back(vi + settings.resolution.y() + 2);
    	        indices.push_back(vi + settings.resolution.y() + 1);
    	    }
    	}
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
template <VertexConfig PV, VkIndexType PI> requires (Bitmask::HasFlag(PV, Position)) && (PI != VK_INDEX_TYPE_NONE_KHR)
void Shape<V, I>::CreateCylinder()
{
	if (settings.resolution.x() < 3) {settings.resolution.x() = 3;}
	if (settings.resolution.y() < 1) {settings.resolution.y() = 1;}

	Shape<V, I> plane(ShapeType::Plane, settings);

	plane.Rotate(90.0, Axis::x);

	Join(plane);

	std::vector<indexType> startVertices;
	std::vector<indexType> endVertices;

	indexType index = 0;

	for (Vertex<V> &vertex : vertices)
	{
		float x = round((vertex.position.x() + 0.5) * settings.resolution.x());

		if (x == 0) {startVertices.push_back(index);}
		else if (x == settings.resolution.x()) {endVertices.push_back(index);}

		point3D newPosition = point3D::Rotation(point3D(0.0, (360.0 / (settings.resolution.x() + 1)) * -x, 0.0)).Unitized();
		vertex.position.x() = newPosition.x();
		vertex.position.z() = newPosition.z();

		if constexpr (hasNormal)
		{
			vertex.normal = point3D(newPosition.x(), 0.0, newPosition.z()).Unitized();
		}

		index++;
	}

	for (size_t i = 0; i < startVertices.size() - 1; i++)
	{
		indices.push_back(endVertices[i + 1]);
		indices.push_back(startVertices[i]);
		indices.push_back(endVertices[i]);

		indices.push_back(endVertices[i + 1]);
		indices.push_back(startVertices[i + 1]);
		indices.push_back(startVertices[i]);
	}
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
void Shape<V, I>::Rotate(const float& degrees, const Axis& axis, int index)
{
	if (index >= vertices.size()) {return;}

	if constexpr (hasPosition)
	{
		vertices[index].position.Rotate(degrees, axis);
	}

	if constexpr (hasNormal)
	{
		vertices[index].normal.Rotate(degrees, axis);
		vertices[index].normal.Unitize();
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Scale(const point3D& scalar, bool scaleUV)
{
	for (Vertex<V>& vertex : vertices)
	{
		if constexpr (hasPosition)
		{
			vertex.position *= scalar;
		}

		if constexpr (hasCoordinate)
		{
			if (scaleUV)
			{
				vertex.coordinate *= scalar;
			}
		}
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Scale(const point3D& scalar, int index)
{
	if (index >= vertices.size()) {return;}

	if constexpr (hasPosition)
	{
		vertices[index].position *= scalar;
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::SetColor(const point3D& color)
{
	if constexpr (hasColor)
	{
		for (Vertex<V>& vertex : vertices)
		{
			vertex.color = color;
		}
	}
}

SHAPE_TEMPLATE
void Shape<V, I>::Paint(const point3D& color)
{
	if constexpr (hasColor)
	{
		for (Vertex<V>& vertex : vertices)
		{
			vertex.color *= color;
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
void Shape<V, I>::Join(const Shape<V, I>& other, bool offset)
{
	const indexType count = (offset ? static_cast<indexType>(vertices.size()) : 0);

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

SHAPE_TEMPLATE
void Shape<V, I>::CalculateNormal()
{
	if constexpr (hasNormal)
	{
		if constexpr (hasPosition)
		{
			for (Vertex<V>& vertex : vertices)
			{
				vertex.normal = point3D(vertex.position).Unitized();
			}
		}
	}
}