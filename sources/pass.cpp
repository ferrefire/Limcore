#include "pass.hpp"

#include "manager.hpp"
#include "utilities.hpp"

#include <stdexcept>

std::vector<VkAttachmentReference> PassConfig::GetColorReferences()
{
	std::vector<VkAttachmentReference> references;

	for (AttachmentConfig& attachment : colorAttachments) references.push_back(attachment.reference);

	return (references);
}

std::vector<VkAttachmentDescription> PassConfig::GetAttachments()
{
	std::vector<VkAttachmentDescription> attachments;

	for (AttachmentConfig& attachment : colorAttachments) attachments.push_back(attachment.description);

	if (depthAttachment.view) attachments.push_back(depthAttachment.description);

	return (attachments);
}

std::vector<VkImageView> PassConfig::GetViews()
{
	std::vector<VkImageView> views;

	for (AttachmentConfig& attachment : colorAttachments) views.push_back(attachment.view);

	if (depthAttachment.view) views.push_back(depthAttachment.view);

	return (views);
}

Pass::Pass()
{

}

Pass::~Pass()
{
	Destroy();
}

void Pass::Create(const PassConfig& passConfig, Device* passDevice = nullptr)
{
	config = passConfig;
	device = passDevice;

	if (!device) device = &Manager::GetDevice();

	CreateRenderPass();
	CreateFramebuffers();
}

void Pass::CreateRenderPass()
{
	if (renderpass) throw (std::runtime_error("Render pass already exists"));
	if (!device) throw (std::runtime_error("Pass has no device"));

	std::vector<VkAttachmentDescription> attachments = config.GetAttachments();

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = CUI(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = CUI(config.subpasses.size());
	createInfo.pSubpasses = config.subpasses.data();

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
		std::vector<VkImageView> views = config.GetViews();
		views[0] = swapchainViews[i];

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderpass;
		createInfo.attachmentCount = CUI(views.size());
		createInfo.pAttachments = views.data();
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

	for (int i = 0; i < framebuffers.size(); i++) 
		vkDestroyFramebuffer(device->GetLogicalDevice(), framebuffers[i], nullptr);
	framebuffers.clear();

	if (renderpass)
	{
		vkDestroyRenderPass(device->GetLogicalDevice(), renderpass, nullptr);
		renderpass = nullptr;
	}

	//std::cout << "Pass destroyed" << std::endl;
}

const VkRenderPass& Pass::GetRenderpass() const
{
	if (!renderpass) throw (std::runtime_error("Renderpass requested but not yet created"));

	return (renderpass);
}

void Pass::Begin(VkCommandBuffer commandBuffer, uint32_t renderIndex)
{
	if (!commandBuffer) throw (std::runtime_error("Cannot begin pass because the command buffer does not exist"));
	if (!renderpass) throw (std::runtime_error("Render pass does not exist"));
	if (renderIndex >= framebuffers.size()) throw (std::runtime_error("Render index is out of range"));
	if (state != Ended) throw (std::runtime_error("Pass has not yet ended"));

	std::vector<VkClearValue> clearValues(1);
	clearValues[0].color = {{0.75f, 0.75f, 0.75f, 1.0f}};

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

VkAttachmentDescription Pass::DefaultColorAttachment()
{
	VkAttachmentDescription description{};
	description.format = VK_FORMAT_B8G8R8A8_UNORM;
	description.samples = VK_SAMPLE_COUNT_1_BIT;
	description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return (description);
}

AttachmentConfig Pass::DefaultAttachmentConfig()
{
	AttachmentConfig config{};
	config.description = DefaultColorAttachment();
	config.reference.attachment = 0;
	config.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	return (config);
}

PassConfig Pass::DefaultConfig()
{
	PassConfig config{};
	config.colorAttachments.push_back(DefaultAttachmentConfig());
	config.subpasses.resize(1);
	config.subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	config.subpasses[0].colorAttachmentCount = 1;
	config.subpasses[0].pColorAttachments = &config.colorAttachments[0].reference;

	return (config);
}