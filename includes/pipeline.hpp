#pragma once

#include "device.hpp"
#include "mesh.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

/**
 * @file pipeline.hpp
 * @brief Vulkan graphics pipeline configuration and wrapper.
 *
 * @details
 * Provides a configuration struct and a Pipeline class that encapsulates
 * the creation, binding, and destruction of Vulkan graphics pipelines.
 * Supports customizable shader stages, vertex input, rasterization, blending,
 * depth/stencil state, and dynamic states.
 */

enum class PipelineType { Graphics, Compute, Shadow };

/** @brief Configuration for creating a Vulkan graphics pipeline. */
struct PipelineConfig
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInput{};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	VkPipelineTessellationStateCreateInfo tesselationState{};
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

	PipelineType type = PipelineType::Graphics;
	bool tesselation = false;
};

/**
 * @brief Vulkan graphics pipeline wrapper.
 *
 * @details
 * Encapsulates a graphics pipeline and its layout. Provides methods
 * for creation, binding, and destruction, and a static helper for
 * generating default configuration templates.
 *
 * Typical usage:
 * - Fill a @ref PipelineConfig with desired state and shader stages.
 * - Call @ref Create() with the config.
 * - Bind the pipeline inside a render pass with @ref Bind().
 * - Destroy resources with @ref Destroy() when no longer needed.
 */
class Pipeline
{
	private:
		PipelineConfig config{};
		Device* device = nullptr;

		VkPipelineLayout layout = nullptr;
		VkPipeline pipeline = nullptr;

		VkShaderModule CreateShader(const std::vector<char>& code);
		void CreateGraphicsConfig();
		void CreateComputeConfig();
		void CreateTesselationConfig();
		void CreateConfig();
		void CreateLayout();
		void CreateGraphicsPipeline();
		void CreateComputePipeline();
		void CreatePipeline();

	public:
		Pipeline(); /**< @brief Constructs an empty pipeline. */
		~Pipeline(); /**< @brief Destroys the pipeline and layout if created. */

		/**
		 * @brief Creates the pipeline from a configuration.
		 * @param pipelineConfig Configuration describing all pipeline states.
		 * @param pipelineDevice Device to use; if @c nullptr, uses the current Device.
		 */		
		void Create(const PipelineConfig& pipelineConfig, Device* pipelineDevice = nullptr);

		/** @brief Destroys the pipeline and layout. */
		void Destroy();

		/**
		 * @brief Gets the Vulkan pipeline layout.
		 * @return Const reference to the @c VkPipelineLayout.
		 */
		const VkPipelineLayout& GetLayout() const;

		const PipelineConfig& GetConfig() const;

		/**
		 * @brief Binds the pipeline to a command buffer.
		 * @param commandBuffer Command buffer to record the bind into.
		 */
		void Bind(VkCommandBuffer commandBuffer);

		/**
		 * @brief Provides a default pipeline configuration.
		 * @return A @ref PipelineConfig with common defaults filled in.
		 */
		static PipelineConfig DefaultConfig();

		static void CreateLayout(VkPipelineLayout* layout, std::vector<VkDescriptorSetLayout> descriptorLayouts, Device* device = nullptr);
};