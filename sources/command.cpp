#include "command.hpp"

#include "manager.hpp"
#include "utilities.hpp"

#include <stdexcept>

Command::Command()
{

}

Command::~Command()
{
	Destroy();
}

void Command::Create(const CommandConfig& commandConfig, Device* commandDevice = nullptr)
{
	config = commandConfig;
	device = commandDevice;

	if (!device) device = &Manager::GetDevice();

	CreatePool();
	AllocateBuffer();
}

void Command::CreatePool()
{
	if (pool) throw (std::runtime_error("Command pool already exists"));
	if (!device) throw (std::runtime_error("Command has no device"));

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = config.queueIndex;
	createInfo.flags = config.poolUsage;

	if (vkCreateCommandPool(device->GetLogicalDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create command pool"));
}

void Command::AllocateBuffer()
{
	if (buffer) throw (std::runtime_error("Command buffer is already allocated"));
	if (!pool) throw (std::runtime_error("Command has no pool"));
	if (!device) throw (std::runtime_error("Command has no device"));

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = pool;
	allocateInfo.level = config.level;
	allocateInfo.commandBufferCount = config.count;

	if (vkAllocateCommandBuffers(device->GetLogicalDevice(), &allocateInfo, &buffer) != VK_SUCCESS)
		throw (std::runtime_error("Failed to allocate command buffer"));
}

void Command::Destroy()
{
	if (!device) return;

	if (pool)
	{
		vkDestroyCommandPool(device->GetLogicalDevice(), pool, nullptr);
		pool = nullptr;
		buffer = nullptr;
	}

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

	if (vkEndCommandBuffer(buffer) != VK_SUCCESS) throw (std::runtime_error("Failed to end command"));

	state = Ended;
}

void Command::Submit()
{
	if (!buffer) throw (std::runtime_error("Command has no buffer"));
	if (!device) throw (std::runtime_error("Command has no device"));
	if (state != Ended) throw (std::runtime_error("Command has not ended yet"));

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