#include "image.hpp"

#include "manager.hpp"
#include "bitmask.hpp"
#include "command.hpp"
#include "buffer.hpp"

#include <stdexcept>

Image::Image()
{

}

Image::~Image()
{
	Destroy();
}

void Image::Create(const ImageConfig& imageConfig, Device* imageDevice)
{
	config = imageConfig;
	device = imageDevice;

	if (!device) device = &Manager::GetDevice();

	CreateImage();
	AllocateMemory();
	CreateView();
	CreateSampler();
	TransitionLayout();
}

void Image::Create(const ImageLoader& imageLoader, const ImageConfig& imageConfig, Device* imageDevice)
{
	config = imageConfig;
	device = imageDevice;

	if (!device) device = &Manager::GetDevice();

	config.width = imageLoader.GetInfo().startOfFrameInfo.width;
	config.height = imageLoader.GetInfo().startOfFrameInfo.height;

	if (config.createMipmaps)
	{
		config.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(config.width, config.height)))) + 1;
		config.targetLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		config.usage = Bitmask::SetFlag(config.usage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		config.usage = Bitmask::SetFlag(config.usage, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		config.viewConfig.subresourceRange.levelCount = config.mipLevels;
		config.samplerConfig.lodRange = point2D(0, VK_LOD_CLAMP_NONE);
	}

	CreateImage();
	AllocateMemory();
	CreateView();
	CreateSampler();
	TransitionLayout();

	Load(imageLoader);

	if (config.createMipmaps) CreateMipmaps();
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
	createInfo.initialLayout = config.currentLayout;

	if (vkCreateImage(device->GetLogicalDevice(), &createInfo, nullptr, &image) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create image"));
}

void Image::CreateMipmaps()
{
	VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), config.format, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("Cannont create image mipmaps because it's format does not support linear blitting");

	Command command;
	CommandConfig commandConfig{};
	commandConfig.queueIndex = device->GetQueueIndex(QueueType::Graphics);
	command.Create(commandConfig, device);
	command.Begin();

	VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = config.viewConfig.subresourceRange.aspectMask;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = config.width;
	int32_t mipHeight = config.height;

	for (uint32_t i = 1; i < config.mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command.GetBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = config.viewConfig.subresourceRange.aspectMask;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = config.viewConfig.subresourceRange.aspectMask;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(command.GetBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command.GetBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
    	if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = config.mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command.GetBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	command.End();
	command.Submit();

	config.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	config.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Image::CreateView()
{
	Image::CreateView(view, image, config.viewConfig, device);
}

void Image::CreateSampler()
{
	if (!Bitmask::HasFlag(config.usage, VK_IMAGE_USAGE_SAMPLED_BIT)) return;
	if (sampler) throw (std::runtime_error("Image sampler already exists"));
	if (!device) throw (std::runtime_error("Device does not exist"));

	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = config.samplerConfig.magFilter;
	createInfo.minFilter = config.samplerConfig.minFilter;
	createInfo.mipmapMode = config.samplerConfig.mipmapMode;
	createInfo.addressModeU = config.samplerConfig.repeatMode;
	createInfo.addressModeV = config.samplerConfig.repeatMode;
	createInfo.addressModeW = config.samplerConfig.repeatMode;
	createInfo.mipLodBias = config.samplerConfig.lodBias;
	createInfo.anisotropyEnable = config.samplerConfig.anisotropyEnabled;
	createInfo.maxAnisotropy = config.samplerConfig.maxAnisotropy;
	createInfo.compareEnable = config.samplerConfig.compareEnabled;
	createInfo.compareOp = config.samplerConfig.compareOperation;
	createInfo.minLod = config.samplerConfig.lodRange.x();
	createInfo.maxLod = config.samplerConfig.lodRange.y();
	createInfo.borderColor = config.samplerConfig.borderColor;
	createInfo.unnormalizedCoordinates = config.samplerConfig.unnormalizedCoordinates;

	if (vkCreateSampler(device->GetLogicalDevice(), &createInfo, nullptr, &sampler) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create image sampler"));
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

	if (sampler)
	{
		vkDestroySampler(device->GetLogicalDevice(), sampler, nullptr);
		sampler = nullptr;
	}
}

VkImage& Image::GetImage()
{
	if (!image) throw (std::runtime_error("Image requested but not yet created"));

	return (image);
}

const VkImageView& Image::GetView() const
{
	if (!view) throw (std::runtime_error("Image view requested but not yet created"));

	return (view);
}

const VkSampler& Image::GetSampler() const
{
	if (!sampler) throw (std::runtime_error("Image sampler requested but not yet created"));

	return (sampler);
}

const ImageConfig& Image::GetConfig() const
{
	return (config);
}

void Image::TransitionLayout()
{
	if (config.currentLayout == config.targetLayout) return;
	if (!image) throw (std::runtime_error("Image does not exist"));
	if (!device) throw (std::runtime_error("Image has no device"));
	//Check if src and dst are supported

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.oldLayout = config.currentLayout;
	barrier.newLayout = config.targetLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange = config.viewConfig.subresourceRange;
	barrier.srcAccessMask = transitionAccesses[config.currentLayout];
	barrier.dstAccessMask = transitionAccesses[config.targetLayout];

	VkPipelineStageFlags srcStage = transitionStages[config.currentLayout];
	VkPipelineStageFlags dstStage = transitionStages[config.targetLayout];

	Command command;
	CommandConfig commandConfig{};
	commandConfig.queueIndex = device->GetQueueIndex(QueueType::Graphics);
	command.Create(commandConfig, device);
	command.Begin();

	vkCmdPipelineBarrier(command.GetBuffer(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	command.End();
	command.Submit();

	config.currentLayout = config.targetLayout;
}

void Image::Load(const ImageLoader& imageLoader)
{
	if (!image) throw (std::runtime_error("Image does not exist"));
	if (!device) throw (std::runtime_error("Image has no device"));

	std::vector<unsigned char> pixels{};
	imageLoader.LoadPixelsThreaded(pixels);
	Update(&pixels[0], pixels.size(), {config.width, config.height, config.depth});
}

void Image::Update(unsigned char* data, size_t size, Point<uint32_t, 3> extent, Point<int32_t, 3> offset)
{
	if (!image) throw (std::runtime_error("Image does not exist"));
	if (!device) throw (std::runtime_error("Image has no device"));

	if (extent.x() == 0 && extent.y() == 0 && extent.z() == 0) extent = {config.width, config.height, 1};

	VkImageLayout originalLayout = config.currentLayout;
	config.targetLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	TransitionLayout();

	Buffer stagingBuffer;
	BufferConfig stagingConfig = Buffer::StagingConfig();
	stagingConfig.size = size;
	stagingBuffer.Create(stagingConfig, data, device);
	stagingBuffer.CopyTo(*this, extent, offset);
	stagingBuffer.Destroy();

	config.targetLayout = originalLayout;

	TransitionLayout();
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

ImageConfig Image::DefaultConfig()
{
	ImageConfig config{};
	config.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	config.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	config.viewConfig = Image::DefaultViewConfig();

	return (config);
}

ImageConfig Image::DefaultStorageConfig()
{
	ImageConfig config{};
	config.format = VK_FORMAT_R16_UNORM;
	config.targetLayout = VK_IMAGE_LAYOUT_GENERAL;
	config.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	config.viewConfig = Image::DefaultViewConfig();
	config.viewConfig.format = config.format;

	return (config);
}

ImageConfig Image::DefaultNormalConfig()
{
	ImageConfig config{};
	config.format = VK_FORMAT_R8G8B8A8_UNORM;
	config.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	config.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	config.viewConfig = Image::DefaultViewConfig();
	config.viewConfig.format = config.format;

	return (config);
}

ImageConfig Image::DefaultGreyscaleConfig()
{
	ImageConfig config{};
	config.format = VK_FORMAT_R8_UNORM;
	config.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	config.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	config.viewConfig = Image::DefaultViewConfig();
	config.viewConfig.format = VK_FORMAT_R8_UNORM;

	return (config);
}

ImageConfig Image::DefaultDepthConfig()
{
	ImageConfig config{};
	config.format = VK_FORMAT_D32_SFLOAT;
	config.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	config.width = Manager::GetWindow().GetConfig().extent.width;
	config.height = Manager::GetWindow().GetConfig().extent.height;
	config.targetLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	config.viewConfig = Image::DefaultViewConfig();
	config.viewConfig.format = config.format;
	config.viewConfig.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	return (config);
}

void Image::CreateView(VkImageView& view, const VkImage& image, const ImageViewConfig& config, Device* device)
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

std::map<VkImageLayout, VkAccessFlags> Image::transitionAccesses =
	{
		std::pair<VkImageLayout, VkAccessFlags>{VK_IMAGE_LAYOUT_UNDEFINED, 0},
		std::pair<VkImageLayout, VkAccessFlags>{VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
		std::pair<VkImageLayout, VkAccessFlags>{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT},
		std::pair<VkImageLayout, VkAccessFlags>{VK_IMAGE_LAYOUT_GENERAL, 0}, // Check if correct later
	};

std::map<VkImageLayout, VkPipelineStageFlags> Image::transitionStages = 
	{
		std::pair<VkImageLayout, VkPipelineStageFlags>{VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
		std::pair<VkImageLayout, VkPipelineStageFlags>{VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT},
		std::pair<VkImageLayout, VkPipelineStageFlags>{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT},
		std::pair<VkImageLayout, VkPipelineStageFlags>{VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT}, // Check if correct later
	};