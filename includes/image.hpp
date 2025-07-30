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

struct ImageConfig
{
	VkImageType type = VK_IMAGE_TYPE_2D;
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t depth = 1;
	uint32_t mipLevels = 1;
	uint32_t arrayLayers = 1;
	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL; 

	ImageViewConfig viewConfig{};
};

class Image
{
	private:
		ImageConfig config{};
		Device* device = nullptr;

		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkDeviceMemory memory = nullptr;

		void CreateImage();
		void CreateView();
		void AllocateMemory();

	public:
		Image();
		~Image();

		void Create(const ImageConfig& imageConfig, Device* imageDevice);

		void Destroy();

		VkImageView& GetView();

		static ImageViewConfig DefaultViewConfig();
		static ImageConfig DefaultDepthConfig();
		static void CreateView(VkImageView& view, const VkImage& image, const ImageViewConfig& config, Device* device);
};