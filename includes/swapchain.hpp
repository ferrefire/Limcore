#pragma once

#include "window.hpp"
#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

class Swapchain
{
	private:
		Window* window = nullptr;
		Device* device = nullptr;

		VkSwapchainKHR swapchain = nullptr;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		uint32_t frameCount = 0;

		void CreateSwapchain();
		void RetrieveImages();
		void CreateViews();

	public:
		Swapchain();
		~Swapchain();

		void Create(Window* swapchainWindow, Device* swapchainDevice);

		void Destroy();

		const VkSwapchainKHR& GetSwapchain() const;
		const uint32_t GetFrameCount() const;
		const std::vector<VkImage>& GetImages();
		const std::vector<VkImageView>& GetViews();
};

std::ostream& operator<<(std::ostream& out, Swapchain& swapchain);