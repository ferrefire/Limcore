#pragma once

#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

/**
 * @file command.hpp
 * @brief Command buffer configuration and wrapper for Vulkan.
 *
 * @details
 * Provides a configuration struct and a Command class that manages
 * command pools, command buffers, and their lifecycle for submission
 * to Vulkan queues. Supports synchronization with fences and semaphores.
 */

/** @brief A Configuration for creating a Command. */
struct CommandConfig
{
	uint32_t count = 1; /**< @brief Number of command buffers to allocate. */
	uint32_t queueIndex = 0; /**< @brief Index of the queue family to submit to. */
	VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; /**< @brief Usage flags (e.g., one-time, simultaneous). */
	VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; /**< @brief Command buffer level (primary or secondary). */
	VkCommandPoolCreateFlags poolUsage = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; /**< @brief Command pool creation flags. */
	VkFence fence = VK_NULL_HANDLE; /**< @brief Optional fence to signal upon completion. */
	std::vector<VkSemaphore> waitSemaphores; /**< @brief Semaphores to wait on before execution. */
	std::vector<VkSemaphore> signalSemaphores; /**< @brief Semaphores to signal after execution. */
	std::vector<VkPipelineStageFlags> waitDestinations; /**< @brief Pipeline stages to wait at for each wait semaphore. */
	bool wait = true; /**< @brief Whether to block until execution finishes. */
};
/**
 * @brief Vulkan command buffer wrapper.
 *
 * @details
 * Manages a command pool and buffer, providing lifecycle management
 * (create/destroy), begin/end recording, and submission with optional
 * synchronization via semaphores and fences.
 * 
 * Typical usage:
 * - Construct and call @ref Create() with a @ref CommandConfig.
 * - Call @ref Begin(), record commands, then @ref End().
 * - Submit with @ref Submit(), optionally waiting for completion.
 */
class Command
{
	enum State {Idle, Began, Ended};

	private:
		State state = Idle;
		CommandConfig config{};
		Device* device = nullptr;

		VkCommandPool pool = nullptr;
		VkCommandBuffer buffer = nullptr;

		void CreatePool();
		void AllocateBuffer();

	public:
		Command(); /**< @brief Constructs an empty Command object. */
		~Command(); /**< @brief Destroys the command pool and buffer. */

		/**
		 * @brief Creates the command pool and buffer.
		 * @param commandConfig Configuration describing command buffer setup.
		 * @param commandDevice Device to use for allocation; if @c nullptr, uses the stored device.
		 */
		void Create(const CommandConfig& commandConfig, Device* commandDevice = nullptr);

		void Destroy(); /**< @brief Destroys the command pool and buffer. */

		/**
		 * @brief Gets the Vulkan command buffer handle.
		 * @return Const reference to the VkCommandBuffer.
		 */
		const VkCommandBuffer& GetBuffer() const;

		/**
		 * @brief Begins recording commands.
		 * @warning Must be followed by @ref End() before submission.
		 */
		void Begin();

		/**
		 * @brief Ends recording commands.
		 * @pre State must be @c Began.
		 * @post State becomes @c Ended.
		 */
		void End();

		/**
		 * @brief Submits the command buffer for execution.
		 *
		 * @details
		 * Submits the recorded buffer to the queue specified in @ref CommandConfig.
		 * Handles synchronization with semaphores and fences.
		 *
		 * @warning Requires the Command to be in the @c Ended state.
		 */
		void Submit();
};
