#pragma once

#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct ImageViewConfig
{
	VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D;
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkComponentMapping components{};
	VkImageSubresourceRange subresourceRange{};
};


class Image
{
	private:
		

	public:
		Image();
		~Image();

		static ImageViewConfig DefaultViewConfig();
		static void CreateView(VkImageView& view, const VkImage& image, ImageViewConfig config, Device* device);
};