#pragma once

#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

struct WindowConfig
{
	bool fullscreen = true;
	VkSurfaceCapabilitiesKHR capabilities{};
	VkSurfaceFormatKHR format{};
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	VkExtent2D extent{};
};

class Window
{
	private:
		uint32_t width = 1600;
		uint32_t height = 900;
		GLFWwindow* data = nullptr;
		VkSurfaceKHR surface = nullptr;
		WindowConfig config{};

		void SelectFormat(Device& device);
		void SelectPresentMode(Device& device);

	public:
		Window();
		~Window();

		void SetConfig(const WindowConfig& windowConfig);

		void CreateFrame();
		void CreateSurface(Device& device);

		void DestroyFrame();
		void DestroySurface();

		GLFWwindow* GetData() const;
		const VkSurfaceKHR& GetSurface() const;
		const WindowConfig& GetConfig() const;

		bool CanPresent(Device& device, int presentQueueIndex);
};

std::ostream& operator<<(std::ostream& out, Window& window);