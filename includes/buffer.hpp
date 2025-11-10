#pragma once

#include "device.hpp"
#include "image.hpp"
#include "point.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

/**
 * @file buffer.hpp
 * @brief Buffer wrapper and configuration utilities for Vulkan.
 *
 * @details
 * Provides a configuration struct and class that manages VkBuffer objects, memory allocation, and data upload.
 * Supports staging, vertex, and index buffers with convenience defaults.
 */

/** @brief Configuration for creating a Buffer. */
struct BufferConfig
{
	bool mapped = false; /**< @brief Whether the buffer should be persistently mapped to host memory. */
	VkDeviceSize size = 0; /**< @brief Size of the buffer in bytes. */
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; /**< @brief Usage flags (e.g., uniform buffer, image sampler). */
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; /**< @brief Memory properties required. */
	VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE; /**< @brief Queue family sharing mode (exclusive or concurrent). */
};

/**
 * @brief Vulkan buffer wrapper.
 *
 * @details
 * Manages creation, destruction, and data transfer for a VkBuffer and its
 * backing VkDeviceMemory. Provides helper methods for updating buffer contents
 * and copying to other buffers or images.
 */
class Buffer
{
	private:
		BufferConfig config{};
		Device* device = nullptr;

		VkBuffer buffer = nullptr;
		VkDeviceMemory memory = nullptr;
		void* address = nullptr;

		void CreateBuffer();
		void AllocateMemory();

	public:
		Buffer(); /**< @brief Constructs an empty buffer object. */
		~Buffer(); /**< @brief Destroys the buffer and releases associated resources. */

		/**
		 * @brief Creates the buffer and optionally uploads initial data.
		 * @param bufferConfig Configuration describing size, usage, and memory properties.
		 * @param data Optional pointer to initial data to upload.
		 * @param bufferDevice Optional device; if @c nullptr, uses the internally stored device.
		 */
		void Create(const BufferConfig& bufferConfig, void* data = nullptr, Device* bufferDevice = nullptr);

		void Destroy(); /**< @brief Destroys the buffer and frees its memory. */

		/**
		 * @brief Checks if the buffer has been created.
		 * @return True if a valid VkBuffer is associated with this object.
		 */
		const bool Created() const;

		/**
		 * @brief Gets the configuration used to create this buffer.
		 * @return Reference to the buffer configuration.
		 */
		const BufferConfig& GetConfig() const;

		/**
		 * @brief Gets the Vulkan buffer handle.
		 * @return Reference to the VkBuffer.
		 */
		const VkBuffer& GetBuffer() const;

		/**
		 * @brief Gets the host address of mapped memory (if mapped).
		 * @return Pointer to the mapped memory region, or @c nullptr if unmapped.
		 */
		const void* GetAddress() const;

		/**
		 * @brief Copies buffer contents into another buffer.
		 * @param target Destination buffer handle.
		 * @param offset Offset in the destination buffer (in bytes).
		 */
		void CopyTo(VkBuffer target, size_t offset = 0);


		/**
		 * @brief Copies buffer contents into an image.
		 * @param target Target image object.
		 * @param extent Dimensions of the region to copy.
		 * @param offset Offset within the target image.
		 */
		void CopyTo(Image& target, Point<uint32_t, 3> extent = {}, Point<int32_t, 4> offset = {});

		/**
		 * @brief Updates the contents of the buffer with new data.
		 * @param data Pointer to the data to copy into the buffer.
		 * @param size Number of bytes to copy.
		 */
		void Update(void* data, size_t size);

		/**
		 * @brief Returns a default configuration for staging buffers.
		 * @return BufferConfig with host-visible, coherent properties.
		 */
		static BufferConfig StagingConfig();

		/**
		 * @brief Returns a default configuration for vertex buffers.
		 * @return BufferConfig with appropriate usage flags for vertices.
		 */
		static BufferConfig VertexConfig();

		/**
		 * @brief Returns a default configuration for index buffers.
		 * @return BufferConfig with appropriate usage flags for indices.
		 */
		static BufferConfig IndexConfig();
};

std::ostream& operator<<(std::ostream& out, const Buffer& buffer);