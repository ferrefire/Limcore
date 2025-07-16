#include "buffer.hpp"

#include "manager.hpp"
#include "printer.hpp"
#include "command.hpp"
#include "bitmask.hpp"

#include <stdexcept>
#include <cstring>

Buffer::Buffer()
{

}

Buffer::~Buffer()
{
	Destroy();
}

void Buffer::Create(const BufferConfig& bufferConfig, Device* bufferDevice = nullptr, void* data = nullptr)
{
	config = bufferConfig;
	device = bufferDevice;

	if (!device) device = &Manager::GetDevice();

	CreateBuffer();
	AllocateMemory();

	if (data != nullptr)
	{
		if (config.mapped && address != nullptr)
		{
			memcpy(address, data, static_cast<size_t>(config.size));
		}
		else if (!config.mapped && Bitmask::HasFlag(config.properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		{
			Buffer stagingBuffer;
			BufferConfig stagingConfig = Buffer::StagingConfig();
			stagingConfig.size = config.size;
			stagingBuffer.Create(stagingConfig, device, data);
			stagingBuffer.CopyTo(buffer);
			stagingBuffer.Destroy();
		}
	}
}

void Buffer::CreateBuffer()
{
	if (buffer) throw (std::runtime_error("Buffer already exists"));
	if (!device) throw (std::runtime_error("Buffer has no device"));

	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = config.size;
	createInfo.usage = config.usage;
	createInfo.sharingMode = config.sharingMode;

	if (vkCreateBuffer(device->GetLogicalDevice(), &createInfo, nullptr, &buffer) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create buffer"));
}

void Buffer::AllocateMemory()
{
	if (memory) throw (std::runtime_error("Memory is already allocated"));
	if (!buffer) throw (std::runtime_error("Buffer does not exist"));
	if (!device) throw (std::runtime_error("Buffer has no device"));

	VkDevice& logicalDevice = device->GetLogicalDevice();

	VkMemoryRequirements requirements{};
	vkGetBufferMemoryRequirements(logicalDevice, buffer, &requirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = requirements.size;
	allocateInfo.memoryTypeIndex = device->FindMemoryType(requirements.memoryTypeBits, config.properties);

	if (vkAllocateMemory(logicalDevice, &allocateInfo, nullptr, &memory) != VK_SUCCESS)
		throw (std::runtime_error("Failed to allocate memory"));

	if (vkBindBufferMemory(logicalDevice, buffer, memory, 0) != VK_SUCCESS)
		throw (std::runtime_error("Failed to bind memory"));

	if (config.mapped)
	{
		if (vkMapMemory(logicalDevice, memory, 0, config.size, 0, &address) != VK_SUCCESS)
			throw (std::runtime_error("Failed to map memory"));
	}
}

void Buffer::Destroy()
{
	if (!device) return;

	if (buffer)
	{
		vkDestroyBuffer(device->GetLogicalDevice(), buffer, nullptr);
		buffer = nullptr;
	}

	if (memory)
	{
		if (address)
		{
			vkUnmapMemory(device->GetLogicalDevice(), memory);
			address = nullptr;
		}

		vkFreeMemory(device->GetLogicalDevice(), memory, nullptr);
		memory = nullptr;
	}
}

const bool Buffer::Created() const
{
	return (buffer != nullptr && memory != nullptr);
}

const BufferConfig& Buffer::GetConfig() const
{
	return (config);
}

const VkBuffer& Buffer::GetBuffer() const
{
	if (!buffer) throw (std::runtime_error("Buffer requested but not yet created"));

	return (buffer);
}

const void* Buffer::GetAddress() const
{
	if (!config.mapped) throw (std::runtime_error("Requested buffer address but buffer is not mapped"));

	return (address);
}

void Buffer::CopyTo(VkBuffer target)
{
	if (!buffer) throw (std::runtime_error("Buffer does not exist"));
	if (!target) throw (std::runtime_error("Buffer copy target does not exist"));
	if (!device) throw (std::runtime_error("Buffer has no device"));

	Command command;
	CommandConfig commandConfig{};
	commandConfig.queueIndex = device->GetQueueIndex(QueueType::Graphics);
	command.Create(commandConfig, device);
	command.Begin();

	VkBufferCopy copyInfo{};
	copyInfo.size = config.size;

	vkCmdCopyBuffer(command.GetBuffer(), buffer, target, 1, &copyInfo);

	command.End();
	command.Submit();
}

void Buffer::Update(void *data, size_t size = 0)
{
	if (!config.mapped) throw (std::runtime_error("Cannot update buffer because it is not mapped"));
	if (!data) throw (std::runtime_error("Cannot update buffer because data does not exist"));
	if (!address) throw (std::runtime_error("Buffer address does not exist"));

	if (size == 0) size = static_cast<size_t>(config.size);
	else size = std::min(static_cast<size_t>(config.size), size);

	memcpy(address, data, size);
}

BufferConfig Buffer::StagingConfig()
{
	BufferConfig config{};
	config.mapped = true;
	config.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	config.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	return (config);
}

BufferConfig Buffer::VertexConfig()
{
	BufferConfig config{};
	config.mapped = false;
	config.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	config.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return (config);
}

BufferConfig Buffer::IndexConfig()
{
	BufferConfig config{};
	config.mapped = false;
	config.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	config.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return (config);
}

std::ostream& operator<<(std::ostream& out, const Buffer& buffer)
{
	BufferConfig config = buffer.GetConfig();

	out << std::endl;
	out << FLAG_VAL(config.usage, VkBufferUsageFlagBits) << std::endl;
	out << VAR_VAL(config.size) << std::endl;
	out << FLAG_VAL(config.properties, VkMemoryPropertyFlagBits) << std::endl;
	out << ENUM_VAL(config.sharingMode) << std::endl;
	out << VAR_VAL(config.mapped) << std::endl;

	return (out);
}