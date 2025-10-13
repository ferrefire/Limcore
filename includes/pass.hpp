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

/** @brief Per-attachment configuration for a Vulkan render pass / framebuffer. */
struct AttachmentConfig
{
	std::vector<VkImageView> views; /**< @brief Image view used by the attachment (e.g., swapchain image or depth image). */
	VkAttachmentDescription description{}; /**< @brief Attachment description (format, load/store ops, layouts, etc.). */
	VkAttachmentReference reference{}; /**< @brief Reference used by subpasses to bind this attachment. */
	VkClearValue clear{}; /**< @brief Clear color/depth/stencil value applied when loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR. */
};

/** @brief Configuration describing a render pass: attachments, subpasses, and options. */
struct PassConfig
{
	std::vector<AttachmentConfig> colorAttachments; /**< @brief Color attachments used by the render pass (one per render target). */
	AttachmentConfig depthAttachment{}; /**< @brief Optional depth/stencil attachment configuration. */
	std::vector<VkSubpassDescription> subpasses; /**< @brief Subpass descriptions defining the rendering pipeline stages. */
	bool depth = false; /**< @brief Whether a depth attachment is used. */

	/**
	 * @brief Gets @c VkAttachmentReference for all color attachments.
	 * @return Vector of color attachment references in the same order as @ref colorAttachments.
	 */
	std::vector<VkAttachmentReference> GetColorReferences();

	/**
	 * @brief Gets all attachment descriptions (color + optional depth) for render pass creation.
	 * @return Vector of @c VkAttachmentDescription matching the attachments used by the pass.
	 */
	std::vector<VkAttachmentDescription> GetAttachments();

	/**
	 * @brief Gets the image views of all configured attachments.
	 * @return Vector of @c VkImageView for color attachments (and depth if present).
	 */
	std::vector<VkImageView> GetViews(size_t frame = 0);

	/**
	 * @brief Gets clear values for all configured attachments.
	 * @return Vector of @c VkClearValue aligned with the attachment order returned by @ref GetAttachments().
	 */
	std::vector<VkClearValue> GetClears();
};

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
		Pass(); /**< @brief Constructs an empty pass (no Vulkan objects created yet). */
		~Pass(); /**< @brief Destroys the pass and any owned Vulkan resources if not already destroyed. */

		/**
		 * @brief Creates the render pass, images, and framebuffers.
		 * @param passConfig Configuration describing attachments and subpasses.
		 * @param passDevice Device used to create Vulkan objects. if @c nullptr, uses the current Device.
		 */
		void Create(const PassConfig& passConfig, Device* passDevice = nullptr);

		/** @brief Destroys the render pass and associated resources. */
		void Destroy();

		/**
		 * @brief Accesses the underlying @c VkRenderPass handle.
		 * @return Const reference to the Vulkan render pass.
		 */
		const VkRenderPass& GetRenderpass() const;

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
		static VkAttachmentDescription DefaultColorDescription();

		/**
		 * @brief Convenience default depth/stencil attachment description.
		 * @return A @c VkAttachmentDescription configured for depth (and stencil if applicable).
		 */
		static VkAttachmentDescription DefaultDepthDescription();

		/**
		 * @brief Builds a default @ref PassConfig.
		 * @param depth Whether to include a depth attachment.
		 * @return A basic configuration suitable for a single-subpass render pass.
		 */
		static PassConfig DefaultConfig(bool depth = false);
};