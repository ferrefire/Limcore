#pragma once

#include "device.hpp"
#include "point.hpp"
#include "loader.hpp"
#include "command.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <string>

/**
 * @file image.hpp
 * @brief Image configuration and wrapper for Vulkan textures.
 *
 * @details
 * Provides configuration structures for sampler, view, and image creation,
 * as well as the @ref Image class for managing Vulkan images, views, and samplers.
 * Supports resource creation, layout transitions, updates from loaders, and defaults
 * for common use cases (e.g. depth images).
 */

/**
 * @brief Describes an Image sampler configuration.
 */
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

/**
 * @brief Describes an Image view configuration.
 */
struct ImageViewConfig
{
	VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D;
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkComponentMapping components{};
	VkImageSubresourceRange subresourceRange{};
};

/**
 * @brief Describes an Image configuration.
 */
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
	VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout targetLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool createMipmaps = false;
	bool srgb = false;
	bool compressed = false;
	bool normal = false;

	ImageViewConfig viewConfig{};
	ImageSamplerConfig samplerConfig{};
};

/**
 * @brief Vulkan image wrapper.
 *
 * @details
 * Encapsulates a Vulkan image, view, and sampler, handling creation,
 * memory allocation, layout transitions, and updates from image data loaders.
 * 
 * Typical usage:
 * - Create an image with @ref Create().
 * - Optionally modify it with @ref Update().
 * - Use @ref GetImage(), @ref GetView(), @ref GetSampler() for rendering.
 * - Destroy resources with @ref Destroy() when no longer needed.
 */
class Image
{
	private:
		static std::map<VkImageLayout, VkAccessFlags> transitionAccesses;
		static std::map<VkImageLayout, VkPipelineStageFlags> transitionStages;

		ImageConfig config{};
		Device* device = nullptr;

		VkImage image = nullptr;
		VkImageView view = nullptr;
		VkSampler sampler = nullptr;
		VkDeviceMemory memory = nullptr;

		void CreateImage();
		void CreateMipmaps();
		void CreateCompressedMipmaps(const ImageLoader& imageLoader);
		void CreateView();
		void CreateSampler();
		void AllocateMemory();

	public:
		Image();
		~Image();

		/**
		 * @brief Creates an image from a configuration.
		 * @param imageConfig Image configuration (format, dimensions, usage, etc.).
		 * @param imageDevice Device to use for creation; if @c nullptr, uses the stored device.
		 */
		void Create(const ImageConfig& imageConfig, Device* imageDevice = nullptr);

		/**
		 * @brief Creates an image and uploads data from an image loader.
		 * @param imageLoader Loader providing pixel data.
		 * @param imageConfig Image configuration.
		 * @param imageDevice Device to use for creation; if @c nullptr, uses the stored device.
		 */
		void Create(const ImageLoader& imageLoader, const ImageConfig& imageConfig, Device* imageDevice = nullptr);

		/** @brief Destroys and frees the Image. */
		void Destroy();

		VkImage& GetImage();
		const VkImage& GetImage() const;
		const VkImageView& GetView() const;
		const VkSampler& GetSampler() const;
		const ImageConfig& GetConfig() const;

		void TransitionLayout();

		/**
		 * @brief Loads pixel data into the image using an image loader.
		 * @param imageLoader Loader containing pixel data.
		 */
		void Load(const ImageLoader& imageLoader);

		/**
		 * @brief Updates the image contents with raw data.
		 * @param data Pointer to pixel data.
		 * @param size Size of the data in bytes.
		 * @param extent Dimensions of the region to update (defaults to full image).
		 * @param offset Offset into the image (defaults to zero).
		 */
		void Update(unsigned char* data, size_t size, Point<uint32_t, 3> extent = {}, Point<int32_t, 4> offset = {}, bool transition = true);

		void CopyTo(Image& target, Command& command, bool signal = true);

		/**
		 * @brief Provides a default image view configuration.
		 * @return ImageViewConfig with common defaults.
		 */
		static ImageViewConfig DefaultViewConfig();

		static ImageConfig DefaultConfig();
		static ImageConfig DefaultStorageConfig();
		static ImageConfig DefaultNormalConfig();
		static ImageConfig DefaultGreyscaleConfig();

		/**
		 * @brief Provides a default configuration for depth images.
		 * @return ImageConfig suitable for depth attachments.
		 */
		static ImageConfig DefaultDepthConfig();

		/**
		 * @brief Creates an image view for an existing Vulkan image.
		 * @param view Output view handle.
		 * @param image Vulkan image to create a view for.
		 * @param config Configuration for the image view.
		 * @param device Device used to create the view; if @c nullptr, uses the stored device.
		 */
		static void CreateView(VkImageView& view, const VkImage& image, const ImageViewConfig& config, Device* device = nullptr);
};