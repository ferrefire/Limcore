#include "image.hpp"

#include "manager.hpp"

#include <stdexcept>

ImageViewConfig Image::DefaultViewConfig()
{
	ImageViewConfig config{};
	config.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	config.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	config.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	config.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	config.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	config.subresourceRange.baseMipLevel = 0;
	config.subresourceRange.levelCount = 1;
	config.subresourceRange.baseArrayLayer = 0;
	config.subresourceRange.layerCount = 1;

	return (config);
}

void Image::CreateView(VkImageView& view, const VkImage& image, ImageViewConfig config, Device* device = nullptr)
{
	if (!device) device = &Manager::GetDevice();

	if (view) throw (std::runtime_error("Image view already exists"));
	if (!image) throw (std::runtime_error("Image does not exist"));
	if (!device) throw (std::runtime_error("Device does not exist"));

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = config.type;
	createInfo.format = config.format;
	createInfo.components = config.components;
	createInfo.subresourceRange = config.subresourceRange;

	if (vkCreateImageView(device->GetLogicalDevice(), &createInfo, nullptr, &view) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create image view"));
}