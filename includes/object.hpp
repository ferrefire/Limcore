#pragma once

#include "shape.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "matrix.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"
#include "image.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct UniformData
{
	mat4 model;
	mat4 view;
	mat4 projection;
	point4D viewPosition;
};

class Object
{
	private:
		Pipeline* pipeline = nullptr;
		Descriptor* descriptor = nullptr;
		meshPNC32* mesh = nullptr;
		Image* image = nullptr;
		
		UniformData data;
		Buffer buffer;
		size_t set;

	public:
		Object();
		~Object();

		void Create(meshPNC32& objectMesh, Image& objectImage, Pipeline& objectPipeline, Descriptor& objectDescriptor);

		void Destroy();

		UniformData& GetData();
		const size_t& GetSet() const;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);
};