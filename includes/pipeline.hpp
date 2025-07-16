#pragma once

#include "device.hpp"
#include "mesh.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

struct PipelineConfig
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInput{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkPipelineTessellationStateCreateInfo tesselation{};
	VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineRasterizationStateCreateInfo rasterization{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineDynamicStateCreateInfo dynamics{};

	std::string shader = "";
	VertexInfo vertexInfo{};
	std::vector<VkDescriptorSetLayout> descriptorLayouts;
	VkRenderPass renderpass = nullptr;
	uint32_t subpass = 0;
	std::vector<VkDynamicState> dynamicStates;
	VkViewport viewport{};
	VkRect2D scissor{};
};

class Pipeline
{
	private:
		PipelineConfig config{};
		Device* device = nullptr;

		VkPipelineLayout layout = nullptr;
		VkPipeline pipeline = nullptr;

		VkShaderModule CreateShader(const std::vector<char>& code);
		void CreateConfig();
		void CreateLayout();
		void CreatePipeline();

	public:
		Pipeline();
		~Pipeline();

		void Create(const PipelineConfig& pipelineConfig, Device* pipelineDevice);

		void Destroy();

		const VkPipelineLayout& GetLayout() const;

		void Bind(VkCommandBuffer commandBuffer);

		static PipelineConfig DefaultConfig();
};