#pragma once

#include "device.hpp"
#include "buffer.hpp"
#include "image.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

/**
 * @file descriptor.hpp
 * @brief Descriptor configuration and wrapper classes for Vulkan.
 *
 * @details
 * Provides configuration structures and a Descriptor class that manages
 * Vulkan descriptor set layouts, pools, and sets. Includes helpers for
 * binding and updating resources such as buffers and images.
 */

/** @brief Configuration for a single descriptor binding. */
struct DescriptorConfig
{
	VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; /**< @brief Type of descriptor (e.g., uniform buffer, sampler). */
	VkShaderStageFlags stages = VK_SHADER_STAGE_ALL; /**< @brief Shader stages that can access this binding. */
	uint32_t count = 1; /**< @brief Number of descriptors (array size). */
};

/**
 * @brief Vulkan descriptor set manager.
 *
 * @details
 * Manages descriptor set layouts, pools, and allocation of descriptor sets.
 * Provides methods to bind descriptor sets to command buffers and update
 * descriptor contents with buffers or images.
 * 
 * Typical usage:
 * - Configure bindings with a vector of @ref DescriptorConfig.
 * - Call @ref Create() to build layout and allocate pool.
 * - Acquire sets with @ref GetNewSet(), then @ref Update() with resources.
 * - Bind sets during command recording with @ref Bind().
 */
class Descriptor
{
	private:
		std::vector<DescriptorConfig> config{};
		Device* device = nullptr;

        VkDescriptorSetLayout layout = nullptr;
		VkDescriptorPool pool = nullptr;
		std::vector<VkDescriptorSet> sets;

		void CreateLayout();
		void CreatePool();
		void AllocateSet(VkDescriptorSet& set);

    public:
        Descriptor(); /**< @brief Constructs an empty Descriptor. */
        ~Descriptor(); /**< @brief Destroys the descriptor pool, layout, and sets. */

		/**
		 * @brief Creates the descriptor set layout and pool.
		 * @param descriptorConfig Vector of descriptor binding configurations.
		 * @param descriptorDevice Device to use for creation; if @c nullptr, uses the stored device.
		 */
		void Create(const std::vector<DescriptorConfig>& descriptorConfig, Device* descriptorDevice = nullptr);

		void Destroy(); /**< @brief Destroys the descriptor resources (layout, pool, sets). */

		/**
		 * @brief Gets the current descriptor configuration.
		 * @return Const reference to the vector of DescriptorConfig.
		 */
		const std::vector<DescriptorConfig>& GetConfig() const;

		/**
		 * @brief Gets the Vulkan descriptor set layout handle.
		 * @return Const reference to the VkDescriptorSetLayout.
		 */
		const VkDescriptorSetLayout& GetLayout() const;

		/**
		 * @brief Allocates and returns the index of a new descriptor set.
		 * @return Index of the new set to be used for other functions.
		 */
		size_t GetNewSet();

		/**
		 * @brief Binds a descriptor set to a command buffer.
		 * @param setID Index of the descriptor set to bind.
		 * @param commandBuffer Command buffer to record the bind into.
		 * @param pipelineLayout Pipeline layout that the descriptor set is compatible with.
		 */
		void Bind(size_t setID, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

		/**
		 * @brief Updates a descriptor set binding with buffer or image info.
		 * @param setID Index of the descriptor set to update.
		 * @param binding Binding index within the descriptor set.
		 * @param bufferInfo Optional buffer descriptor info (if updating a buffer binding).
		 * @param imageInfo Optional image descriptor info (if updating an image binding).
		 */
		void Update(size_t setID, uint32_t binding, VkDescriptorBufferInfo* bufferInfo = nullptr, VkDescriptorImageInfo* imageInfo = nullptr);

		/**
		 * @brief Updates a descriptor set binding with a buffer resource.
		 * @param setID Index of the descriptor set to update.
		 * @param binding Binding index within the descriptor set.
		 * @param buffer Buffer object to bind.
		 */
		void Update(size_t setID, uint32_t binding, const Buffer& buffer);

		/**
		 * @brief Updates a descriptor set binding with an image resource.
		 * @param setID Index of the descriptor set to update.
		 * @param binding Binding index within the descriptor set.
		 * @param image Image object to bind.
		 */
		void Update(size_t setID, uint32_t binding, const Image& image);
};

std::ostream& operator<<(std::ostream& out, const DescriptorConfig& config);
std::ostream& operator<<(std::ostream& out, const Descriptor& descriptor);