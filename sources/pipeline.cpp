#include "pipeline.hpp"

#include "manager.hpp"
#include "utilities.hpp"

#include <stdexcept>
#include <string>

Pipeline::Pipeline()
{

}

Pipeline::~Pipeline()
{
	Destroy();
}

void Pipeline::Create(const PipelineConfig& pipelineConfig, Device* pipelineDevice = nullptr)
{
	config = pipelineConfig;
	device = pipelineDevice;

	if (!device) device = &Manager::GetDevice();

	CreateConfig();
	CreateLayout();
	CreatePipeline();
}

VkShaderModule Pipeline::CreateShader(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule = nullptr;
	if (vkCreateShaderModule(device->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader");

	return (shaderModule);
}

void Pipeline::CreateConfig()
{
	if (config.shader == "") throw std::runtime_error("Pipeline config has no shader");

	std::string path = Utilities::GetPath();

	VkShaderModule vertexShader = CreateShader(Utilities::FileToBinary(path + "/shaders/" + config.shader + ".vert.spv"));
	VkShaderModule fragmentShader = CreateShader(Utilities::FileToBinary(path + "/shaders/" + config.shader + ".frag.spv"));

	config.shaderStages.resize(2);

	config.shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	config.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	config.shaderStages[0].module = vertexShader;
	config.shaderStages[0].pName = "main";

	config.shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	config.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	config.shaderStages[1].module = fragmentShader;
	config.shaderStages[1].pName = "main";

	config.vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	config.vertexInput.vertexBindingDescriptionCount = config.vertexInfo.bindingCount;
	config.vertexInput.pVertexBindingDescriptions = &config.vertexInfo.bindingDescription;
	config.vertexInput.vertexAttributeDescriptionCount = config.vertexInfo.attributeCount;
	config.vertexInput.pVertexAttributeDescriptions = config.vertexInfo.attributeDescriptions.data();

	config.dynamics.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	config.dynamics.dynamicStateCount = CUI(config.dynamicStates.size());
	config.dynamics.pDynamicStates = (config.dynamicStates.size() > 0 ? config.dynamicStates.data() : nullptr);

	if (!Utilities::Contains(config.dynamicStates, VK_DYNAMIC_STATE_VIEWPORT)) config.viewportState.pViewports = &config.viewport;
	if (!Utilities::Contains(config.dynamicStates, VK_DYNAMIC_STATE_SCISSOR)) config.viewportState.pScissors = &config.scissor;
}

void Pipeline::CreateLayout()
{
	if (layout) throw (std::runtime_error("Pipeline layout already exists"));
	if (!device) throw (std::runtime_error("Pipeline has no device"));

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = CUI(config.descriptorLayouts.size());
	createInfo.pSetLayouts = config.descriptorLayouts.data();

	if (vkCreatePipelineLayout(device->GetLogicalDevice(), &createInfo, nullptr, &layout) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create pipeline layout"));
}

void Pipeline::CreatePipeline()
{
	if (pipeline) throw (std::runtime_error("Pipeline already exists"));
	if (!layout) throw (std::runtime_error("Pipeline has no layout"));
	if (!device) throw (std::runtime_error("Pipeline has no device"));

	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = CUI(config.shaderStages.size());
	createInfo.pStages = config.shaderStages.data();
	createInfo.pVertexInputState = &config.vertexInput;
	createInfo.pInputAssemblyState = &config.inputAssembly;
	//createInfo.pTessellationState = &config.tesselation;
	createInfo.pViewportState = &config.viewportState;
	createInfo.pRasterizationState = &config.rasterization;
	createInfo.pMultisampleState = &config.multisampling;
	createInfo.pDepthStencilState = &config.depthStencil;
	createInfo.pColorBlendState = &config.colorBlending;
	createInfo.pDynamicState = &config.dynamics;
	createInfo.layout = layout;
	createInfo.renderPass = config.renderpass;
	createInfo.subpass = config.subpass;

	if (vkCreateGraphicsPipelines(device->GetLogicalDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create pipeline"));

	for (VkPipelineShaderStageCreateInfo& shaderStage : config.shaderStages)
		vkDestroyShaderModule(device->GetLogicalDevice(), shaderStage.module, nullptr);

	config.shaderStages.clear();
}

void Pipeline::Destroy()
{
	if (!device) return;

	if (pipeline)
	{
		vkDestroyPipeline(device->GetLogicalDevice(), pipeline, nullptr);
		pipeline = nullptr;
	}

	if (layout)
	{
		vkDestroyPipelineLayout(device->GetLogicalDevice(), layout, nullptr);
		layout = nullptr;
	}

	for (VkPipelineShaderStageCreateInfo& shaderStage : config.shaderStages)
		vkDestroyShaderModule(device->GetLogicalDevice(), shaderStage.module, nullptr);

	config.shaderStages.clear();

	//std::cout << "Pipeline destroyed" << std::endl;
}

const VkPipelineLayout& Pipeline::GetLayout() const
{
	if (!layout) throw (std::runtime_error("Pipeline layout requested but does not exist"));

	return (layout);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	if (!commandBuffer) throw (std::runtime_error("Can't bind pipeline because the command buffer does not exist"));
	if (!pipeline) throw (std::runtime_error("Pipeline does not exist"));

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

PipelineConfig Pipeline::DefaultConfig()
{
	PipelineConfig config{};

	config.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	config.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	config.inputAssembly.primitiveRestartEnable = VK_FALSE;

	config.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	config.viewportState.viewportCount = 1;
	config.viewportState.scissorCount = 1;

	config.rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	config.rasterization.depthClampEnable = VK_FALSE;
	config.rasterization.rasterizerDiscardEnable = VK_FALSE;
	config.rasterization.polygonMode = VK_POLYGON_MODE_FILL;
	config.rasterization.lineWidth = 1.0f;
	config.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
	config.rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	config.rasterization.depthBiasEnable = VK_FALSE;
	config.rasterization.depthBiasConstantFactor = 0.0f;
	config.rasterization.depthBiasClamp = 0.0f;
	config.rasterization.depthBiasSlopeFactor = 0.0f;

	config.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	config.multisampling.sampleShadingEnable = VK_FALSE;
	config.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	config.multisampling.minSampleShading = 1.0f;
	config.multisampling.pSampleMask = nullptr;
	config.multisampling.alphaToCoverageEnable = VK_FALSE;
	config.multisampling.alphaToOneEnable = VK_FALSE;

	config.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	config.depthStencil.depthTestEnable = VK_TRUE;
	config.depthStencil.depthWriteEnable = VK_TRUE;
	config.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	config.depthStencil.depthBoundsTestEnable = VK_FALSE;
	config.depthStencil.minDepthBounds = 0.0f;
	config.depthStencil.maxDepthBounds = 1.0f;
	config.depthStencil.stencilTestEnable = VK_FALSE;

	config.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	config.colorBlendAttachment.blendEnable = VK_FALSE;

	config.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	config.colorBlending.logicOpEnable = VK_FALSE;
	config.colorBlending.logicOp = VK_LOGIC_OP_COPY;
	config.colorBlending.attachmentCount = 1;
	config.colorBlending.pAttachments = &config.colorBlendAttachment;
	config.colorBlending.blendConstants[0] = 0.0f;
	config.colorBlending.blendConstants[1] = 0.0f;
	config.colorBlending.blendConstants[2] = 0.0f;
	config.colorBlending.blendConstants[3] = 0.0f;

	config.viewport.x = 0.0f;
	config.viewport.y = 0.0f;
	config.viewport.width = Manager::GetWindow().GetConfig().extent.width;
	config.viewport.height = Manager::GetWindow().GetConfig().extent.height;
	config.viewport.minDepth = 0.0f;
	config.viewport.maxDepth = 1.0f;

	config.scissor.offset = {0, 0};
	config.scissor.extent = Manager::GetWindow().GetConfig().extent;

	return (config);
}