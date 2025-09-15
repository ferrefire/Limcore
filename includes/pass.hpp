#pragma once

#include "device.hpp"
#include "image.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

struct AttachmentConfig
{
	VkImageView view = nullptr;
	VkAttachmentDescription description{};
	VkAttachmentReference reference{};
	VkClearValue clear{};
};

struct PassConfig
{
	std::vector<AttachmentConfig> colorAttachments;
	AttachmentConfig depthAttachment{};
	std::vector<VkSubpassDescription> subpasses;
	bool depth = false;

	std::vector<VkAttachmentReference> GetColorReferences(); 
	std::vector<VkAttachmentDescription> GetAttachments();
	std::vector<VkImageView> GetViews();
	std::vector<VkClearValue> GetClears();
};

class Pass
{
	enum State {Began, Ended};

	private:
		State state = Ended;
		PassConfig config{};
		Device* device = nullptr;

		VkRenderPass renderpass = nullptr;
		std::vector<Image*> images;
		std::vector<VkFramebuffer> framebuffers;

		void CreateRenderPass();
		void CreateImages();
		void CreateFramebuffers();

		void DestroyImages();
		void DestroyFramebuffers();

	public:
		Pass();
		~Pass();

		void Create(const PassConfig& passConfig, Device* passDevice);

		void Destroy();

		const VkRenderPass& GetRenderpass() const;

		void Begin(VkCommandBuffer commandBuffer, uint32_t renderIndex);
		void End(VkCommandBuffer commandBuffer);

		void Recreate();

		static VkAttachmentDescription DefaultColorDescription();
		static VkAttachmentDescription DefaultDepthDescription();
		static PassConfig DefaultConfig(bool depth = false);
};