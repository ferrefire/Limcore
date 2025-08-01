#include "image.hpp"

#include "manager.hpp"

#include <stdexcept>

Image::Image()
{

}

Image::~Image()
{
	Destroy();
}

void Image::Create(const ImageConfig& imageConfig, Device* imageDevice = nullptr)
{
	config = imageConfig;
	device = imageDevice;

	if (!device) device = &Manager::GetDevice();

	CreateImage();
	AllocateMemory();
	CreateView();
}

void Image::CreateImage()
{
	if (image) throw (std::runtime_error("Image already exists"));
	if (!device) throw (std::runtime_error("Image has no device"));

	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = config.type;
	createInfo.format = config.format;
	createInfo.extent.width = config.width;
	createInfo.extent.height = config.height;
	createInfo.extent.depth = config.depth;
	createInfo.mipLevels = config.mipLevels;
	createInfo.arrayLayers = config.arrayLayers;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = config.usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.initialLayout = config.initialLayout;

	if (vkCreateImage(device->GetLogicalDevice(), &createInfo, nullptr, &image) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create image"));
}

void Image::CreateView()
{
	Image::CreateView(view, image, config.viewConfig, device);
}

void Image::AllocateMemory()
{
	if (memory) throw (std::runtime_error("Image memory already exists"));
	if (!image) throw (std::runtime_error("Image does not exist"));
	if (!device) throw (std::runtime_error("Image has no device"));

	VkMemoryRequirements requirements{};
	vkGetImageMemoryRequirements(device->GetLogicalDevice(), image, &requirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = device->FindMemoryType(requirements.memoryTypeBits, config.properties);

	if (vkAllocateMemory(device->GetLogicalDevice(), &allocateInfo, nullptr, &memory) != VK_SUCCESS)
		throw (std::runtime_error("Failed to allocate memory"));

	if (vkBindImageMemory(device->GetLogicalDevice(), image, memory, 0) != VK_SUCCESS)
		throw (std::runtime_error("Failed to bind memory"));
}

void Image::Destroy()
{
	if (!device) return;

	if (image)
	{
		vkDestroyImage(device->GetLogicalDevice(), image, nullptr);
		image = nullptr;
	}

	if (memory)
	{
		vkFreeMemory(device->GetLogicalDevice(), memory, nullptr);
		memory = nullptr;
	}

	if (view)
	{
		vkDestroyImageView(device->GetLogicalDevice(), view, nullptr);
		view = nullptr;
	}
}

VkImageView& Image::GetView()
{
	if (!view) throw (std::runtime_error("Image view requested but not yet created"));

	return (view);
}

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

ImageConfig Image::DefaultDepthConfig()
{
	ImageConfig config{};
	config.format = VK_FORMAT_D32_SFLOAT;
	config.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	config.width = Manager::GetWindow().GetConfig().extent.width;
	config.height = Manager::GetWindow().GetConfig().extent.height;
	config.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	config.viewConfig = Image::DefaultViewConfig();
	config.viewConfig.format = config.format;
	config.viewConfig.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	return (config);
}

void Image::CreateView(VkImageView& view, const VkImage& image, const ImageViewConfig& config, Device* device = nullptr)
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