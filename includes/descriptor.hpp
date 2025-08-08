#pragma once

#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

struct DescriptorConfig
{
	VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	VkShaderStageFlags stages = VK_SHADER_STAGE_ALL;
	uint32_t count = 1;
};

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
        Descriptor();
        ~Descriptor();

		void Create(const std::vector<DescriptorConfig>& descriptorConfig, Device* descriptorDevice);

		void Destroy();

		const std::vector<DescriptorConfig>& GetConfig() const;
		const VkDescriptorSetLayout& GetLayout() const;

		size_t GetNewSet();
		void Bind(size_t setID, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
		void Update(size_t setID, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo);
};

std::ostream& operator<<(std::ostream& out, const DescriptorConfig& config);
std::ostream& operator<<(std::ostream& out, const Descriptor& descriptor);