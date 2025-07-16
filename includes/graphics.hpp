#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class Graphics
{
	private:
		static VkInstance instance;

		static bool validationLayersEnabled;
		static std::vector<const char*> validationLayers;

		static bool HasValidationLayers();

	public:
		Graphics();
		~Graphics();

		static void CreateInstance();

		static void DestroyInstance();

		static const VkInstance& GetInstance();
};