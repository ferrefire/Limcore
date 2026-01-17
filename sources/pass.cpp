#include "pass.hpp"

#include "manager.hpp"
#include "utilities.hpp"

#include <stdexcept>

Pass::Pass()
{

}

Pass::~Pass()
{
	Destroy();
}

//void Pass::Create(const PassConfig& passConfig, Device* passDevice)
void Pass::Create(Device* passDevice)
{
	//config = passConfig;
	device = passDevice;

	if (!device) device = &Manager::GetDevice();

	CreateImages();
	CreateRenderPass();
	CreateFramebuffers();

	Manager::RegisterResizeCall(this, &Pass::Recreate);

	//std::cout << "Pass images: " << images.size() << std::endl;
}

void Pass::CreateImages()
{
	/*if (!config.useSwapchain)
	{
		for (size_t i = 0; i < Manager::GetSwapchain().GetFrameCount(); i++)
		{
			images.push_back(new Image());
			ImageConfig imageConfig = Image::DefaultConfig();
			imageConfig.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			//imageConfig.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageConfig.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			imageConfig.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageConfig.width = Manager::GetWindow().GetConfig().extent.width;
			imageConfig.height = Manager::GetWindow().GetConfig().extent.height;
			imageConfig.viewConfig.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			imageConfig.samplerConfig.minFilter = VK_FILTER_NEAREST;
			imageConfig.samplerConfig.magFilter = VK_FILTER_NEAREST;
			images[images.size() - 1]->Create(imageConfig, device);
			config.colorAttachments[0].images.push_back(images[images.size() - 1]);
			config.colorAttachments[0].views.push_back(images[images.size() - 1]->GetView());
		}
	}

	if (config.depth)
	{
		for (size_t i = 0; i < Manager::GetSwapchain().GetFrameCount(); i++)
		{
			images.push_back(new Image());
			ImageConfig imageConfig = Image::DefaultDepthConfig();
			imageConfig.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			imageConfig.targetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			//imageConfig.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			//imageConfig.targetLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			images[images.size() - 1]->Create(imageConfig, device);
			config.depthAttachment.images.push_back(images[images.size() - 1]);
			config.depthAttachment.views.push_back(images[images.size() - 1]->GetView());
		}
	}*/

	for (size_t i = 0; i < attachments.size(); i++)
	{
		if (attachments[i].useWindowExtent)
		{
			attachments[i].config.width = Manager::GetWindow().GetConfig().extent.width;
			attachments[i].config.height = Manager::GetWindow().GetConfig().extent.height;
		}

		for (size_t j = 0; j < Manager::GetSwapchain().GetFrameCount(); j++)
		{
			if (attachments[i].useSwapchain)
			{
				attachments[i].views.push_back(Manager::GetSwapchain().GetViews()[j]);
			}
			else
			{
				images.push_back(new Image());
				images[images.size() - 1]->Create(attachments[i].config, device);
				attachments[i].images.push_back(images[images.size() - 1]);
				attachments[i].views.push_back(images[images.size() - 1]->GetView());
			}
		}
	}
}

void Pass::CreateRenderPass()
{
	if (renderpass) throw (std::runtime_error("Render pass already exists"));
	if (!device) throw (std::runtime_error("Pass has no device"));

	//config.subpasses[0].pColorAttachments = &config.colorAttachments[0].reference;
	//if (config.depth) config.subpasses[0].pDepthStencilAttachment = &config.depthAttachment.reference;

	//std::vector<VkAttachmentDescription> attachments = config.GetAttachments();
	//std::vector<VkAttachmentDescription> attachments = config.GetAttachments();

	/*VkSubpassDependency dependency = {
    	.srcSubpass = 0,
    	.dstSubpass = 1,
    	.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    	.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    	.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    	.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};*/

	std::vector<VkAttachmentDescription> attachmentDescriptions;
	attachmentDescriptions.resize(attachments.size());
	for (size_t i = 0; i < attachments.size(); i++) {attachmentDescriptions[i] = attachments[i].description;}

	std::vector<VkSubpassDependency> subpassDependencies;

	std::vector<VkSubpassDescription> subpassDescriptions;
	subpassDescriptions.resize(subpasses.size());
	for (size_t i = 0; i < subpasses.size(); i++)
	{
		subpassDescriptions[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[i].colorAttachmentCount = subpasses[i].colorReferences.size();
		subpassDescriptions[i].pColorAttachments = subpassDescriptions[i].colorAttachmentCount > 0 ? subpasses[i].colorReferences.data() : nullptr;
		subpassDescriptions[i].inputAttachmentCount = subpasses[i].inputReferences.size();
		subpassDescriptions[i].pInputAttachments = subpassDescriptions[i].inputAttachmentCount > 0 ? subpasses[i].inputReferences.data() : nullptr;
		subpassDescriptions[i].pDepthStencilAttachment = subpasses[i].useDepth ? &subpasses[i].depthReference : nullptr;

		if (subpasses[i].useDependency) {subpassDependencies.push_back(subpasses[i].dependency);}
	}

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = CUI(attachmentDescriptions.size());
	createInfo.pAttachments = attachmentDescriptions.data();
	createInfo.subpassCount = CUI(subpassDescriptions.size());
	createInfo.pSubpasses = subpassDescriptions.data();
	createInfo.dependencyCount = CUI(subpassDependencies.size());
	createInfo.pDependencies = createInfo.dependencyCount > 0 ? subpassDependencies.data() : nullptr;

	if (vkCreateRenderPass(device->GetLogicalDevice(), &createInfo, nullptr, &renderpass) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create render pass"));
}

void Pass::CreateFramebuffers()
{
	if (framebuffers.size() != 0) throw (std::runtime_error("Frame buffers already exist"));
	if (!renderpass) throw (std::runtime_error("Render pass does not exist yet"));
	if (!device) throw (std::runtime_error("Pass has no device"));

	const std::vector<VkImageView>& swapchainViews = Manager::GetSwapchain().GetViews();

	framebuffers.resize(swapchainViews.size());
	for (int i = 0; i < swapchainViews.size(); i++)
	{
		//std::vector<VkImageView> views = config.GetViews(i);
		//if (config.useSwapchain) views[0] = swapchainViews[i];
		//views[1] = swapchainViews[i];

		std::vector<VkImageView> frameViews;
		frameViews.resize(attachments.size());
		for (size_t j = 0; j < attachments.size(); j++) {frameViews[j] = attachments[j].views[i];}

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderpass;
		createInfo.attachmentCount = CUI(frameViews.size());
		createInfo.pAttachments = frameViews.data();
		createInfo.width = Manager::GetWindow().GetConfig().extent.width;
		createInfo.height = Manager::GetWindow().GetConfig().extent.height;
		createInfo.layers = 1;

		if (vkCreateFramebuffer(device->GetLogicalDevice(), &createInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			throw (std::runtime_error("Failed to create framebuffer"));
	}
}

void Pass::Destroy()
{
	if (!device) return;

	DestroyImages();

	DestroyFramebuffers();

	if (renderpass)
	{
		vkDestroyRenderPass(device->GetLogicalDevice(), renderpass, nullptr);
		renderpass = nullptr;
	}

	//std::cout << "Pass destroyed" << std::endl;
}

void Pass::DestroyImages()
{
	for (Image* image : images)
	{
		image->Destroy();
		delete(image);
	}
	images.clear();

	for (AttachmentConfig& attachment : attachments)
	{
		attachment.views.clear();
		attachment.images.clear();
	}

	//for (AttachmentConfig& attachment : config.colorAttachments)
	//{
	//	attachment.views.clear();
	//	attachment.images.clear();
	//}

	//config.depthAttachment.views.clear();
	//config.depthAttachment.images.clear();
}

void Pass::DestroyFramebuffers()
{
	for (size_t i = 0; i < framebuffers.size(); i++) 
	{
		vkDestroyFramebuffer(device->GetLogicalDevice(), framebuffers[i], nullptr);
	}
	framebuffers.clear();
}

const VkRenderPass& Pass::GetRenderpass() const
{
	if (!renderpass) throw (std::runtime_error("Renderpass requested but not yet created"));

	return (renderpass);
}

const Image* Pass::GetAttachmentImage(size_t attachment, size_t index) const
{
	if (attachment >= attachments.size() || index >= attachments[attachment].images.size()) 
		throw (std::runtime_error("Pass image request index out of bounds"));

	return (attachments[attachment].images[index]);
}

/*const Image* Pass::GetColorImage(size_t index) const
{
	if (config.colorAttachments.size() <= 0 || index >= config.colorAttachments[0].images.size()) 
		throw (std::runtime_error("Pass image request index out of bounds"));

	return (config.colorAttachments[0].images[index]);
}

const Image* Pass::GetDepthImage(size_t index) const
{
	if (index >= config.depthAttachment.images.size()) 
		throw (std::runtime_error("Pass image request index out of bounds"));

	return (config.depthAttachment.images[index]);
}*/

void Pass::AddAttachment(AttachmentConfig attachmentConfig)
{
	attachments.push_back(attachmentConfig);
}

void Pass::AddSubpass(SubpassConfig subpassConfig)
{
	subpasses.push_back(subpassConfig);
}

void Pass::Begin(VkCommandBuffer commandBuffer, uint32_t renderIndex)
{
	if (!commandBuffer) throw (std::runtime_error("Cannot begin pass because the command buffer does not exist"));
	if (!renderpass) throw (std::runtime_error("Render pass does not exist"));
	if (renderIndex >= framebuffers.size()) throw (std::runtime_error("Render index is out of range"));
	if (state != Ended) throw (std::runtime_error("Pass has not yet ended"));

	//std::vector<VkClearValue> clearValues = config.GetClears();

	std::vector<VkClearValue> clearValues;
	clearValues.resize(attachments.size());
	for (size_t i = 0; i < attachments.size(); i++) {clearValues[i] = attachments[i].clear;}

	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = renderpass;
	beginInfo.framebuffer = framebuffers[renderIndex];
	beginInfo.renderArea.offset = {0, 0};
	beginInfo.renderArea.extent = Manager::GetWindow().GetConfig().extent;
	beginInfo.clearValueCount = CUI(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	state = Began;
}

void Pass::End(VkCommandBuffer commandBuffer)
{
	if (!commandBuffer) throw (std::runtime_error("Cannot end pass because the command buffer does not exist"));
	if (state != Began) throw (std::runtime_error("Pass has not yet began"));

	vkCmdEndRenderPass(commandBuffer);

	state = Ended;
}

void Pass::Recreate()
{
	DestroyImages();
	DestroyFramebuffers();

	CreateImages();
	CreateFramebuffers();
}

/*VkAttachmentDescription Pass::DefaultColorDescription()
{
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_R8G8B8A8_UNORM;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	return (description);
}*/

VkAttachmentDescription Pass::DefaultSwapDescription()
{
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_B8G8R8A8_SRGB;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return (description);
}

VkAttachmentDescription Pass::DefaultDepthDescription()
{
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_D32_SFLOAT;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	return (description);
}

VkAttachmentDescription Pass::DefaultHDRDescription()
{
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	return (description);
}

AttachmentConfig Pass::DefaultSwapAttachment()
{
	AttachmentConfig attachmentConfig{};
	attachmentConfig.description = DefaultSwapDescription();
	attachmentConfig.clear = {{0.0f, 0.0f, 0.0f, 1.0f}};
	attachmentConfig.useSwapchain = true;

	return (attachmentConfig);
}

AttachmentConfig Pass::DefaultDepthAttachment(bool input)
{
	AttachmentConfig attachmentConfig{};
	attachmentConfig.description = DefaultDepthDescription();
	attachmentConfig.clear.depthStencil = {1.0f, 0};
	attachmentConfig.config = Image::DefaultDepthConfig();

	if (input)
	{
		attachmentConfig.config.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		attachmentConfig.config.targetLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}

	return (attachmentConfig);
}

AttachmentConfig Pass::DefaultHDRAttachment()
{
	AttachmentConfig attachmentConfig{};
	attachmentConfig.description = DefaultHDRDescription();
	attachmentConfig.clear = {{0.0f, 0.0f, 0.0f, 1.0f}};
	
	ImageConfig imageConfig = Image::DefaultConfig();
	imageConfig.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	imageConfig.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	imageConfig.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageConfig.width = Manager::GetWindow().GetConfig().extent.width;
	imageConfig.height = Manager::GetWindow().GetConfig().extent.height;
	imageConfig.viewConfig.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	//imageConfig.samplerConfig.minFilter = VK_FILTER_NEAREST;
	//imageConfig.samplerConfig.magFilter = VK_FILTER_NEAREST;

	attachmentConfig.config = imageConfig;

	return (attachmentConfig);
}

void SubpassConfig::AddColorReference(uint32_t index, VkImageLayout layout)
{
	VkAttachmentReference reference{};
	reference.attachment = index;
	reference.layout = layout;
	colorReferences.push_back(reference);
}

void SubpassConfig::AddDepthReference(uint32_t index, VkImageLayout layout)
{
	VkAttachmentReference reference{};
	reference.attachment = index;
	reference.layout = layout;
	depthReference = reference;
	useDepth = true;
}

void SubpassConfig::AddInputReference(uint32_t index, VkImageLayout layout)
{
	VkAttachmentReference reference{};
	reference.attachment = index;
	reference.layout = layout;
	inputReferences.push_back(reference);
}

void SubpassConfig::AddDependency(VkSubpassDependency subpassDependency)
{
	dependency = subpassDependency;
	useDependency = true;
}

/*PassConfig Pass::DefaultConfig(bool depth)
{
	AttachmentConfig colorAttachment{};
	colorAttachment.description = DefaultColorDescription();
	colorAttachment.reference.attachment = 0;
	colorAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.clear.color = {{0.75f, 0.75f, 0.75f, 1.0f}};

	AttachmentConfig depthAttachment{};
	depthAttachment.description = DefaultDepthDescription();
	depthAttachment.reference.attachment = 2;
	depthAttachment.reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	depthAttachment.clear.depthStencil = {1.0f, 0};

	PassConfig config{};
	config.depth = depth;
	config.colorAttachments.push_back(colorAttachment);
	config.colorAttachments.push_back(colorAttachment);
	config.colorAttachments[1].reference.attachment = 1;
	if (depth) config.depthAttachment = depthAttachment;
	config.subpasses.resize(2);
	config.subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	config.subpasses[0].colorAttachmentCount = 1;
	config.subpasses[0].pColorAttachments = &config.colorAttachments[0].reference;
	config.subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	config.subpasses[1].colorAttachmentCount = 1;
	config.subpasses[1].pColorAttachments = &config.colorAttachments[1].reference;
	config.subpasses[1].inputAttachmentCount = 2;
	config.subpasses[1].pInputAttachments = config.inputRefs;
	if (depth) config.subpasses[0].pDepthStencilAttachment = &config.depthAttachment.reference;

	return (config);
}*/