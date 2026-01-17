#pragma once

#include "device.hpp"
#include "image.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

/**
 * @file pass.hpp
 * @brief Render pass configuration and wrapper utilities for Vulkan.
 *
 * @details
 * Provides small configs for attachments and passes, plus a @ref Pass
 * class that owns a VkRenderPass, per-swapchain images, and framebuffers.
 * The class manages creation, begin/end, and destruction of these resources.
 */


struct AttachmentConfig
{
	VkAttachmentDescription description{};
	VkClearValue clear{};

	ImageConfig config{};
	std::vector<Image*> images;
	std::vector<VkImageView> views;

	bool useSwapchain = false;
	bool useWindowExtent = true;
};

struct SubpassConfig
{
	//VkSubpassDescription description{};

	std::vector<VkAttachmentReference> colorReferences;
	std::vector<VkAttachmentReference> inputReferences;
	VkAttachmentReference depthReference{};
	VkSubpassDependency dependency{};

	bool useDepth = false;
	bool useDependency = false;

	void AddColorReference(uint32_t index, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	void AddDepthReference(uint32_t index, VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	void AddInputReference(uint32_t index, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	void AddDependency(VkSubpassDependency subpassDependency);
};

/*struct PassConfig
{
	std::vector<AttachmentConfig> attachments;
	std::vector<SubpassConfig> subpasses;
};*/

/**
 * @brief High-level owner for a Vulkan render pass plus its images and framebuffers.
 *
 * @details
 * Manages the lifecycle of a @c VkRenderPass, associated per-target images, and
 * VkFramebuffer objects.
 * 
 * Typical usage:
 * - Configure with @ref PassConfig (or @ref DefaultConfig()).
 * - Call @ref Create().
 * - For each frame: @ref Begin() -> record draw calls -> @ref End().
 * - When done or on resize: @ref Destroy() or @ref Recreate().
 */
class Pass
{
	enum State {Began, Ended};

	private:
		State state = Ended;
		//PassConfig config{};
		Device* device = nullptr;

		std::vector<AttachmentConfig> attachments;
		std::vector<SubpassConfig> subpasses;

		VkRenderPass renderpass = nullptr;
		std::vector<Image*> images;
		std::vector<VkFramebuffer> framebuffers;

		void CreateRenderPass();
		void CreateImages();
		void CreateFramebuffers();

		void DestroyImages();
		void DestroyFramebuffers();

	public:
		Pass(); /**< @brief Constructs an empty pass (no Vulkan objects created yet). */
		~Pass(); /**< @brief Destroys the pass and any owned Vulkan resources if not already destroyed. */

		/**
		 * @brief Creates the render pass, images, and framebuffers.
		 * @param passConfig Configuration describing attachments and subpasses.
		 * @param passDevice Device used to create Vulkan objects. if @c nullptr, uses the current Device.
		 */
		//void Create(const PassConfig& passConfig, Device* passDevice = nullptr);
		void Create(Device* passDevice = nullptr);

		/** @brief Destroys the render pass and associated resources. */
		void Destroy();

		/**
		 * @brief Accesses the underlying @c VkRenderPass handle.
		 * @return Const reference to the Vulkan render pass.
		 */
		const VkRenderPass& GetRenderpass() const;

		const Image* GetAttachmentImage(size_t attachment, size_t index) const;

		void AddAttachment(AttachmentConfig attachmentConfig);
		void AddSubpass(SubpassConfig subpassConfig);

		/**
		 * @brief Begins a render pass on the given command buffer.
		 * @param commandBuffer Command buffer into which the render pass will be recorded.
		 * @param renderIndex Index of the framebuffer/render target to use (e.g., swapchain image index).
		 * @pre State is @c Ended and @ref Create() has been called successfully.
		 * @post State becomes @c Began until @ref End() is called.
		 */
		void Begin(VkCommandBuffer commandBuffer, uint32_t renderIndex);

		/**
		 * @brief Ends the current render pass on the given command buffer.
		 * @param commandBuffer Command buffer that was used in @ref Begin().
		 * @pre State is @c Began .
		 * @post State becomes @c Ended .
		 */
		void End(VkCommandBuffer commandBuffer);

		/**
		 * @brief Recreates render targets and framebuffers (e.g., after a resize).
		 * @details Typically destroys and rebuilds images/framebuffers while preserving the render pass configuration.
		 */
		void Recreate();

		/**
		 * @brief Convenience default color attachment description.
		 * @return A @c VkAttachmentDescription suitable for a typical color render target (implementation chooses sensible load/store ops and layouts).
		 */
		//static VkAttachmentDescription DefaultColorDescription();

		static VkAttachmentDescription DefaultSwapDescription();

		/**
		 * @brief Convenience default depth/stencil attachment description.
		 * @return A @c VkAttachmentDescription configured for depth (and stencil if applicable).
		 */
		static VkAttachmentDescription DefaultDepthDescription();

		static VkAttachmentDescription DefaultHDRDescription();

		static AttachmentConfig DefaultSwapAttachment();
		static AttachmentConfig DefaultDepthAttachment(bool input = false);
		static AttachmentConfig DefaultHDRAttachment();

		/**
		 * @brief Builds a default @ref PassConfig.
		 * @param depth Whether to include a depth attachment.
		 * @return A basic configuration suitable for a single-subpass render pass.
		 */
		//static PassConfig DefaultConfig(bool depth = false);
};