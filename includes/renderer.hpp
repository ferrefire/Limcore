#pragma once

#include "command.hpp"
#include "pass.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "point.hpp"
#include "matrix.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

/**
 * @file renderer.hpp
 * @brief Renderer and frame management for Vulkan.
 *
 * @details
 * Provides a static Renderer class that manages per-frame resources
 * such as fences, semaphores, command buffers, and render passes.
 * Supports adding passes, registering draw calls, and driving the
 * rendering loop with synchronization and presentation.
 */

/** @brief Information about a render pass and its associated commands. */
struct PassInfo
{
	Pass* pass = nullptr; /**< @brief Pointer to a Pass object. */
	std::vector<std::function<void(VkCommandBuffer, uint32_t)>> calls; /**< @brief Registered draw functions to be called for this pass. */
	VkViewport viewport{}; /**< @brief Viewport configuration for this pass. */
	VkRect2D scissor{}; /**< @brief Scissor rectangle configuration. */
	bool useWindowExtent = false; /**< @brief Whether to use swapchain window extent for viewport/scissor. */
};

/**
 * @brief Static renderer for managing frames, synchronization, and passes.
 *
 * @details
 * Handles synchronization objects (fences, semaphores), per-frame command
 * buffers, and a sequence of render passes. Supports recording calls into
 * passes and presenting frames.
 *
 * Typical usage:
 * - Add passes with @ref AddPass() and register draw calls with @ref RegisterCall().
 * - Call @ref Destroy() before shutdown.
 */
class Renderer
{
	private:
		static Device* device;
		static Swapchain* swapchain;

		static uint32_t frameCount;
		static uint32_t currentFrame;
		static uint32_t renderIndex;

		static std::vector<VkFence> fences;
		static std::vector<VkSemaphore> renderSemaphores;
		static std::vector<VkSemaphore> presentSemaphores;
		static std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>> frameSemaphores;
		static std::vector<Command> commands;
		static std::vector<PassInfo> passes;

		static void CreateFences();
		static void CreateSemaphores();
		static void CreateCommands();

		static void RecordCommands();
		static void PresentFrame();

	public:
		Renderer();
		~Renderer();

		static void Create(uint32_t rendererFrameCount, Device* rendererDevice = nullptr, Swapchain* rendererSwapchain = nullptr);

		static void Destroy();

		/**
		 * @brief Accesses information for a render pass.
		 * @param index Index of the pass in the passes list.
		 * @return Reference to the PassInfo.
		 */
		static PassInfo& GetPassInfo(size_t index);
		
		static uint32_t GetFrameCount();
		static uint32_t GetCurrentFrame();

		static void WaitForFrame();
		static void Frame();

		/**
		 * @brief Adds a render pass to the renderer.
		 * @param passInfo Information about the new pass.
		 */
		static void AddPass(PassInfo passInfo);

		/**
		 * @brief Registers a call into a pass.
		 * @param index Pass index.
		 * @param call Function recording Vulkan commands (VkCommandBuffer, render index).
		 */
		static void RegisterCall(size_t index, std::function<void(VkCommandBuffer, uint32_t)> call);

		template <class T>
		static void RegisterCall(size_t index, T* object, void (T::*call)(VkCommandBuffer, uint32_t)) { RegisterCall(index, std::bind_front(call, object)); }
		
		static void Resize();

		static void AddFrameSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitStage);
};