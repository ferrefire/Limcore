#pragma once

#include "command.hpp"
#include "pass.hpp"
#include "device.hpp"
#include "swapchain.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>

struct PassInfo
{
	Pass* pass = nullptr;
	std::vector<std::function<void(VkCommandBuffer, uint32_t)>> calls;
	VkViewport viewport{};
	VkRect2D scissor{};
	bool useWindowExtent = false;
};

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

		static PassInfo& GetPassInfo(size_t index);

		static void Frame();

		static void AddPass(PassInfo passInfo);
		static void RegisterCall(size_t index, std::function<void(VkCommandBuffer, uint32_t)> call);
		
		static void Resize();
};