#pragma once

#include "device.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

struct CommandConfig
{
	uint32_t count = 1;
	uint32_t queueIndex = 0;
	VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VkCommandPoolCreateFlags poolUsage = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkFence fence = VK_NULL_HANDLE;
	std::vector<VkSemaphore> waitSemaphores;
	std::vector<VkSemaphore> signalSemaphores;
	std::vector<VkPipelineStageFlags> waitDestinations;
	bool wait = true;
};

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
		Command();
		~Command();

		void Create(const CommandConfig& commandConfig, Device* commandDevice);

		void Destroy();

		const VkCommandBuffer& GetBuffer() const;

		void Begin();
		void End();
		void Submit();
};
