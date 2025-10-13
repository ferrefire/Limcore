#include "command.hpp"

#include "manager.hpp"
#include "utilities.hpp"
#include "renderer.hpp"

#include <stdexcept>

Command::Command()
{

}

Command::~Command()
{
	Destroy();
}

void Command::Create(const CommandConfig& commandConfig, size_t commandFrame, Device* commandDevice)
{
	config = commandConfig;
	device = commandDevice;
	frame = commandFrame;

	if (!device) device = &Manager::GetDevice();

	//CreatePool();
	AllocateBuffer();
}

void Command::Create(const CommandConfig& commandConfig, Device* commandDevice)
{
	Create(commandConfig, Renderer::GetCurrentFrame(), commandDevice);
}

/*void Command::CreatePool()
{
	if (pool) throw (std::runtime_error("Command pool already exists"));
	if (!device) throw (std::runtime_error("Command has no device"));

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = config.queueIndex;
	createInfo.flags = config.poolUsage;

	if (vkCreateCommandPool(device->GetLogicalDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create command pool"));
}*/

void Command::AllocateBuffer()
{
	if (buffer) throw (std::runtime_error("Command buffer is already allocated"));
	if (pools.size() <= frame) throw (std::runtime_error("Command does not have enough pools"));
	if (!device) throw (std::runtime_error("Command has no device"));

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = pools[frame];
	allocateInfo.level = config.level;
	allocateInfo.commandBufferCount = config.count;

	if (vkAllocateCommandBuffers(device->GetLogicalDevice(), &allocateInfo, &buffer) != VK_SUCCESS)
		throw (std::runtime_error("Failed to allocate command buffer"));
}

void Command::Destroy()
{
	if (!device) return;

	/*if (pool)
	{
		vkDestroyCommandPool(device->GetLogicalDevice(), pool, nullptr);
		pool = nullptr;
		buffer = nullptr;
	}*/

	buffer = nullptr;

	//std::cout << "Buffer destroyed" << std::endl;
}

const VkCommandBuffer& Command::GetBuffer() const
{
	if (!buffer) throw (std::runtime_error("Command buffer requested but does not exist"));

	return (buffer);
}

void Command::Begin()
{
	if (!buffer) throw (std::runtime_error("Command has no buffer"));
	if (state != Idle) throw (std::runtime_error("Command is not idle"));
	if (frame != Renderer::GetCurrentFrame()) throw (std::runtime_error("Command is out of frame"));
	
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = config.usage;

	if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) throw (std::runtime_error("Failed to begin command"));

	state = Began;
}

void Command::End()
{
	if (!buffer) throw (std::runtime_error("Command has no buffer"));
	if (state != Began) throw (std::runtime_error("Command has not yet began"));
	if (frame != Renderer::GetCurrentFrame()) throw (std::runtime_error("Command is out of frame"));

	if (vkEndCommandBuffer(buffer) != VK_SUCCESS) throw (std::runtime_error("Failed to end command"));

	state = Ended;
}

void Command::Submit()
{
	if (!buffer) throw (std::runtime_error("Command has no buffer"));
	if (!device) throw (std::runtime_error("Command has no device"));
	if (state != Ended) throw (std::runtime_error("Command has not ended yet"));
	if (frame != Renderer::GetCurrentFrame()) throw (std::runtime_error("Command is out of frame"));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = CUI(config.waitSemaphores.size());
	submitInfo.pWaitSemaphores = submitInfo.waitSemaphoreCount > 0 ? config.waitSemaphores.data() : nullptr;
	submitInfo.pWaitDstStageMask = submitInfo.waitSemaphoreCount > 0 ? config.waitDestinations.data() : nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;
	submitInfo.signalSemaphoreCount = CUI(config.signalSemaphores.size());
	submitInfo.pSignalSemaphores = submitInfo.signalSemaphoreCount > 0 ? config.signalSemaphores.data() : nullptr;

	if (vkQueueSubmit(device->GetQueue(config.queueIndex), 1, &submitInfo, config.fence) != VK_SUCCESS)
		throw (std::runtime_error("Failed to submit command to a queue"));
	if (config.wait && vkQueueWaitIdle(device->GetQueue(config.queueIndex)) != VK_SUCCESS)
		throw (std::runtime_error("Failed to wait for a queue to become idle"));

	state = Idle;
}

void Command::CreatePools(Device* commandDevice)
{
	if (pools.size() > 0) throw (std::runtime_error("Command pools already exists"));
	if (!commandDevice) commandDevice = &Manager::GetDevice();
	if (!commandDevice) throw (std::runtime_error("Command has no device"));

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = commandDevice->GetQueueIndex(QueueType::Graphics);
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	pools.resize(Manager::GetConfig().framesInFlight);

	for (VkCommandPool& pool : pools)
	{
		if (vkCreateCommandPool(commandDevice->GetLogicalDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS)
			throw (std::runtime_error("Failed to create command pool"));
	}
}

void Command::ResetPool(Device* commandDevice)
{
	if (pools.size() <= 0) return;
	if (!commandDevice) commandDevice = &Manager::GetDevice();
	if (!commandDevice) return;

	vkResetCommandPool(commandDevice->GetLogicalDevice(), pools[Renderer::GetCurrentFrame()], 0);
}

void Command::DestroyPools(Device* commandDevice)
{
	if (pools.size() <= 0) return;
	if (!commandDevice) commandDevice = &Manager::GetDevice();
	if (!commandDevice) return;

	for (VkCommandPool& pool : pools) { vkDestroyCommandPool(commandDevice->GetLogicalDevice(), pool, nullptr); }

	pools.clear();
}

std::vector<VkCommandPool> Command::pools;