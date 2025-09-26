#pragma once

#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

/**
 * @file window.hpp
 * @brief Window and surface management for Vulkan with GLFW.
 *
 * @details
 * Provides a @ref Window class for creating and managing a GLFW window
 * and its associated Vulkan surface. Handles configuration, creation,
 * destruction, resizing, and presentation capability queries.
 */

/** @brief Configuration options for a Vulkan-capable window. */
struct WindowConfig
{
	bool fullscreen = true;
	VkSurfaceCapabilitiesKHR capabilities{};
	VkSurfaceFormatKHR format{};
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkExtent2D extent{};
};

/**
 * @brief GLFW window wrapper with Vulkan surface integration.
 *
 * @details
 * Encapsulates a GLFW window and its associated Vulkan surface. Provides
 * configuration, creation/destruction of both window and surface, and
 * methods to query and handle presentation compatibility with a given device.
 *
 * Typical usage:
 * - Configure via @ref SetConfig() (optional).
 * - Call @ref CreateFrame() to create the GLFW window.
 * - Call @ref CreateSurface() with a @ref Device to make a Vulkan surface.
 * - Query surface and config with @ref GetSurface(), @ref GetConfig().
 * - Call @ref Resize() to handle window resizes.
 * - Destroy resources with @ref DestroyFrame() and @ref DestroySurface().
 */
class Window
{
	private:
		uint32_t width = 1600;
		uint32_t height = 900;
		GLFWwindow* data = nullptr;
		VkSurfaceKHR surface = nullptr;
		WindowConfig config{};

		void SelectFormat(Device& device);
		void SelectPresentMode(Device& device);

	public:
		/** @brief Constructs an empty window wrapper. */
		Window();

		/** @brief Destroys the window and surface if still allocated. */
		~Window();

		/**
		 * @brief Sets the window configuration.
		 * @param windowConfig New window configuration to apply.
		 */
		void SetConfig(const WindowConfig& windowConfig);

		/** @brief Creates the GLFW window frame (but not the Vulkan surface). */
		void CreateFrame();

		/**
		 * @brief Creates a Vulkan surface for the window.
		 * @param device Device to use for surface creation.
		 */
		void CreateSurface(Device& device);

		/** @brief Destroys the GLFW window frame. */
		void DestroyFrame();

		/** @brief Destroys the Vulkan surface. */
		void DestroySurface();

		/**
		 * @brief Gets the underlying GLFW window pointer.
		 * @return Pointer to the GLFWwindow.
		 */
		GLFWwindow* GetData() const;

		/**
		 * @brief Gets the Vulkan surface handle.
		 * @return Const reference to the VkSurfaceKHR.
		 */
		const VkSurfaceKHR& GetSurface() const;

		/**
		 * @brief Gets the current window configuration.
		 * @return Const reference to the WindowConfig.
		 */
		const WindowConfig& GetConfig() const;

		/**
		 * @brief Checks if the device/queue can present to this surface.
		 * @param device Vulkan device to check.
		 * @param presentQueueIndex Queue index to test.
		 * @return True if presentation is supported.
		 */
		bool CanPresent(Device& device, int presentQueueIndex);

		/**
		 * @brief Handles window resizing by updating surface capabilities and extent.
		 * @param device Vulkan device used for queries.
		 */
		void Resize(Device& device);
};

std::ostream& operator<<(std::ostream& out, Window& window);