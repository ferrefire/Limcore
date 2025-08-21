#pragma once

#include "device.hpp"
#include "point.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct ImageSamplerConfig
{
	VkFilter magFilter = VK_FILTER_LINEAR;
	VkFilter minFilter = VK_FILTER_LINEAR;
	VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	VkSamplerAddressMode repeatMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	float lodBias = 0;
	VkBool32 anisotropyEnabled = VK_FALSE;
	float maxAnisotropy = 0;
	VkBool32 compareEnabled = VK_FALSE;
	VkCompareOp compareOperation = VK_COMPARE_OP_ALWAYS;
	point2D lodRange = point2D(0);
	VkBorderColor borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
	VkBool32 unnormalizedCoordinates = VK_FALSE;
};

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
	ImageSamplerConfig samplerConfig{};
};

class Image
{
	private:
		ImageConfig config{};
		Device* device = nullptr;

		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkSampler sampler = nullptr;
		VkDeviceMemory memory = nullptr;

		void CreateImage();
		void CreateView();
		void CreateSampler();
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