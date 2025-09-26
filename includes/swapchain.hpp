#pragma once

#include "window.hpp"
#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

/**
 * @file swapchain.hpp
 * @brief Vulkan swapchain wrapper for window presentation.
 *
 * @details
 * Encapsulates creation and management of a Vulkan swapchain, including
 * retrieval of swapchain images and creation of image views for rendering.
 * Provides methods to create, destroy, and query swapchain resources.
 */

/**
 * @brief Vulkan swapchain manager.
 *
 * @details
 * Manages the lifecycle of a Vulkan swapchain for a specific window/device pair.
 * Handles creation of swapchain images, associated image views, and provides
 * queries for frame count and swapchain handles.
 *
 * Typical usage:
 * - Call @ref Create() after the window and device are ready.
 * - Query @ref GetImages() and @ref GetViews() for rendering targets.
 * - Use @ref GetFrameCount() to determine number of backbuffers.
 * - Call @ref Destroy() before shutdown or when resizing.
 */
class Swapchain
{
	private:
		Window* window = nullptr;
		Device* device = nullptr;

		VkSwapchainKHR swapchain = nullptr;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		uint32_t frameCount = 0;

		void CreateSwapchain();
		void RetrieveImages();
		void CreateViews();

	public:
		/** @brief Constructs an empty swapchain wrapper. */
		Swapchain();

		/** @brief Destroys the swapchain and its associated resources. */
		~Swapchain();

		/**
		 * @brief Creates the swapchain and associated resources.
		 * @param swapchainWindow Target window; if @c nullptr, uses the current window.
		 * @param swapchainDevice Logical device; if @c nullptr, uses the current device.
		 */
		void Create(Window* swapchainWindow = nullptr, Device* swapchainDevice = nullptr);

		/** @brief Destroys the swapchain, image views, and clears state. */
		void Destroy();

		/**
		 * @brief Gets the Vulkan swapchain handle.
		 * @return Const reference to the VkSwapchainKHR.
		 */
		const VkSwapchainKHR& GetSwapchain() const;

		/**
		 * @brief Gets the number of images (frames in flight).
		 * @return Frame count.
		 */
		const uint32_t GetFrameCount() const;

		/**
		 * @brief Gets the swapchain images.
		 * @return Const reference to the image vector.
		 */
		const std::vector<VkImage>& GetImages();

		/**
		 * @brief Gets the image views corresponding to the swapchain images.
		 * @return Const reference to the image view vector.
		 */
		const std::vector<VkImageView>& GetViews();
};

std::ostream& operator<<(std::ostream& out, Swapchain& swapchain);