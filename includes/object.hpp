#pragma once

#include "shape.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "matrix.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"
#include "image.hpp"
#include "renderer.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//struct UniformData
//{
//	mat4 model;
//	mat4 view;
//	mat4 projection;
//	point4D viewPosition;
//};

class Object
{
	private:
		Pipeline* pipeline = nullptr;
		meshPNC32* mesh = nullptr;
		
		//UniformData data;
		//Buffer buffer;
		size_t set;
		size_t materialSet;

	public:
		Object();
		~Object();

		void Create(meshPNC32& objectMesh, Pipeline& objectPipeline);

		void Destroy();

		UniformObjectData& GetData();
		const size_t& GetSet() const;
		const size_t& GetMaterialSet() const;

		void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame);
};