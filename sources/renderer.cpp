#include "renderer.hpp"

#include "manager.hpp"

#include <stdexcept>

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	Destroy();
}

void Renderer::Create(uint32_t rendererFrameCount, Device* rendererDevice, Swapchain* rendererSwapchain)
{
	frameCount = rendererFrameCount;
	device = rendererDevice;
	swapchain = rendererSwapchain;

	if (frameCount <= 0) throw (std::runtime_error("Renderer frame count cannot be zero"));
	if (!device) device = &Manager::GetDevice();
	if (!swapchain) swapchain = &Manager::GetSwapchain();

	CreateFences();
	CreateSemaphores();
	CreateCommands();
}

void Renderer::CreateFences()
{
	if (!device) throw (std::runtime_error("Renderer has no device"));
	if (fences.size() != 0) throw (std::runtime_error("Renderer fences already exist"));

	fences.resize(frameCount);

	for (int i = 0; i < frameCount; i++)
	{
		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(device->GetLogicalDevice(), &createInfo, nullptr, &fences[i]) != VK_SUCCESS)
			throw (std::runtime_error("Failed to create fence"));
	}
}

void Renderer::CreateSemaphores()
{
	if (!device) throw (std::runtime_error("Renderer has no device"));
	if (renderSemaphores.size() != 0) throw (std::runtime_error("Renderer render semaphores already exist"));
	if (presentSemaphores.size() != 0) throw (std::runtime_error("Renderer present semaphores already exist"));

	renderSemaphores.resize(frameCount);
	presentSemaphores.resize(frameCount);

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (int i = 0; i < frameCount; i++)
	{
		if (vkCreateSemaphore(device->GetLogicalDevice(), &createInfo, nullptr, &renderSemaphores[i]) != VK_SUCCESS)
			throw (std::runtime_error("Failed to create render semaphore"));
		if (vkCreateSemaphore(device->GetLogicalDevice(), &createInfo, nullptr, &presentSemaphores[i]) != VK_SUCCESS)
			throw (std::runtime_error("Failed to create present semaphore"));
	}
}

void Renderer::CreateCommands()
{
	if (!device) throw (std::runtime_error("Renderer has no device"));
	if (commands.size() != 0) throw (std::runtime_error("Renderer commands already exist"));
	if (fences.size() == 0) throw (std::runtime_error("Renderer fences do not exist"));
	if (renderSemaphores.size() == 0) throw (std::runtime_error("Renderer render semaphores do not exist"));
	if (presentSemaphores.size() == 0) throw (std::runtime_error("Renderer present semaphores do not exist"));

	commands.resize(frameCount);

	for (int i = 0; i < frameCount; i++)
	{
		CommandConfig commandConfig{};
		commandConfig.queueIndex = device->GetQueueIndex(QueueType::Graphics);
		commandConfig.wait = false;
		commandConfig.fence = fences[i];
		commandConfig.waitSemaphores = {renderSemaphores[i]};
		commandConfig.waitDestinations = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		commandConfig.signalSemaphores = {presentSemaphores[i]};

		commands[i].Create(commandConfig, device);
	}
}

void Renderer::Destroy()
{
	if (!device) return;

	for (VkFence& fence : fences)
	{
		vkDestroyFence(device->GetLogicalDevice(), fence, nullptr);
	}
	fences.clear();

	for (VkSemaphore& semaphore : renderSemaphores)
	{
		vkDestroySemaphore(device->GetLogicalDevice(), semaphore, nullptr);
	}
	renderSemaphores.clear();

	for (VkSemaphore& semaphore : presentSemaphores)
	{
		vkDestroySemaphore(device->GetLogicalDevice(), semaphore, nullptr);
	}
	presentSemaphores.clear();

	for (Command& command : commands)
	{
		command.Destroy();
	}
	commands.clear();
}

PassInfo& Renderer::GetPassInfo(size_t index)
{
	if (passes.size() <= index) throw (std::runtime_error("Pass info requested but it does not exist"));

	return (passes[index]);
}

void Renderer::Frame()
{
	if (passes.size() == 0) return;

	for (const PassInfo& passInfo : passes)
	{
		if (passInfo.pass == nullptr) return;
		if (passInfo.calls.size() == 0) return;
	}

	if (vkWaitForFences(device->GetLogicalDevice(), 1, &fences[currentFrame], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		throw (std::runtime_error("Failed to wait for fence"));

	VkResult acquireResult = vkAcquireNextImageKHR(device->GetLogicalDevice(), swapchain->GetSwapchain(), UINT64_MAX, 
		renderSemaphores[currentFrame], VK_NULL_HANDLE, &renderIndex);

	if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) { Manager::Resize(); return; }
	else if (acquireResult == VK_SUBOPTIMAL_KHR) Manager::Resize(false);
	else if (acquireResult != VK_SUCCESS) throw (std::runtime_error("Failed to aquire next render image index"));

	if (vkResetFences(device->GetLogicalDevice(), 1, &fences[currentFrame]) != VK_SUCCESS)
		throw (std::runtime_error("Failed to reset fence"));

	RecordCommands();
	PresentFrame();

	currentFrame = (currentFrame + 1) % frameCount;
}

void Renderer::RecordCommands()
{
	commands[currentFrame].Begin();

	for (PassInfo& passInfo : passes)
	{
		passInfo.pass->Begin(commands[currentFrame].GetBuffer(), renderIndex);

		vkCmdSetViewport(commands[currentFrame].GetBuffer(), 0, 1, &passInfo.viewport);
		vkCmdSetScissor(commands[currentFrame].GetBuffer(), 0, 1, &passInfo.scissor);

		for (std::function<void(VkCommandBuffer, uint32_t)> call : passInfo.calls)
		{
			call(commands[currentFrame].GetBuffer(), currentFrame);
		}

		passInfo.pass->End(commands[currentFrame].GetBuffer());
	}

	commands[currentFrame].End();

	commands[currentFrame].Submit();
}

void Renderer::PresentFrame()
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentSemaphores[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain->GetSwapchain();
	presentInfo.pImageIndices = &renderIndex;

	VkResult presentResult = vkQueuePresentKHR(device->GetQueue(device->GetQueueIndex(QueueType::Graphics)), &presentInfo);

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) Manager::Resize();
	else if (presentResult == VK_SUBOPTIMAL_KHR) Manager::Resize(false);
	else if (presentResult != VK_SUCCESS) throw (std::runtime_error("Failed to present frame"));
}

void Renderer::AddPass(PassInfo passInfo)
{
	if (!passInfo.pass) throw (std::runtime_error("Cannot add pass because it does not exist"));

	if (passInfo.useWindowExtent)
	{
		passInfo.viewport.x = 0.0f;
		passInfo.viewport.y = 0.0f;
		passInfo.viewport.width = Manager::GetWindow().GetConfig().extent.width;
		passInfo.viewport.height = Manager::GetWindow().GetConfig().extent.height;
		passInfo.viewport.minDepth = 0.0f;
		passInfo.viewport.maxDepth = 1.0f;

		passInfo.scissor.offset = {0, 0};
		passInfo.scissor.extent = Manager::GetWindow().GetConfig().extent;
	}

	passes.push_back(passInfo);
}

void Renderer::RegisterCall(size_t index, std::function<void(VkCommandBuffer, uint32_t)> call)
{
	if (passes.size() <= index) return;

	passes[index].calls.push_back(call);
}

void Renderer::Resize()
{
	swapchain = &Manager::GetSwapchain();

	for (PassInfo& passInfo : passes)
	{
		if (passInfo.useWindowExtent)
		{
			passInfo.viewport.width = Manager::GetWindow().GetConfig().extent.width;
			passInfo.viewport.height = Manager::GetWindow().GetConfig().extent.height;

			passInfo.scissor.extent = Manager::GetWindow().GetConfig().extent;
		}
	}
}

Device* Renderer::device = nullptr;
Swapchain* Renderer::swapchain = nullptr;

uint32_t Renderer::frameCount = 0;
uint32_t Renderer::currentFrame = 0;
uint32_t Renderer::renderIndex = 0;

std::vector<VkFence> Renderer::fences;
std::vector<VkSemaphore> Renderer::renderSemaphores;
std::vector<VkSemaphore> Renderer::presentSemaphores;
std::vector<Command> Renderer::commands;
std::vector<PassInfo> Renderer::passes;