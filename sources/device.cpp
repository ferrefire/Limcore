#include "device.hpp"

#include "graphics.hpp"
#include "utilities.hpp"
#include "manager.hpp"
#include "printer.hpp"

#include <stdexcept>
#include <vector>
#include <set>
#include <string>

Device::Device()
{

}

Device::~Device()
{
	Destroy();
}

void Device::SetConfig(const DeviceConfig& deviceConfig)
{
	config = deviceConfig;
}

void Device::CreatePhysical()
{
	if (physicalDevice) throw (std::runtime_error("Physical device already exists"));

	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(Graphics::GetInstance(), &deviceCount, nullptr);
	if (deviceCount <= 0) throw (std::runtime_error("Failed to find a GPU with Vulkan support"));

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(Graphics::GetInstance(), &deviceCount, physicalDevices.data());

	physicalDevice = GetBestDevice(config).physicalDevice;

	if (!physicalDevice) throw (std::runtime_error("Failed to find a suitable GPU"));
}

void Device::CreateLogical()
{
	if (logicalDevice) throw (std::runtime_error("Logical device already exists"));
	if (!physicalDevice) throw (std::runtime_error("Physical device does not exist"));
	if (queueFamilies.graphicsFamily == -1 ||queueFamilies.presentFamily == -1) throw (std::runtime_error("Queue families not complete"));

	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {CUI(queueFamilies.graphicsFamily), CUI(queueFamilies.presentFamily)};

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamily;
		createInfo.queueCount = 1;
		createInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(createInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	if (config.tesselation) deviceFeatures.tessellationShader = VK_TRUE;
	if (config.anisotropic) deviceFeatures.samplerAnisotropy = VK_TRUE;
	if (config.shaderDouble) deviceFeatures.shaderFloat64 = VK_TRUE;

	std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = CUI(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = CUI(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create logical device"));
}

void Device::SelectQueues()
{
	if (!physicalDevice) throw (std::runtime_error("Physical device does not exist"));

	Window& window = Manager::GetWindow();

	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueFamilyProperties.data());

	for (int i = 0; i < queueCount; i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (queueFamilies.graphicsFamily == -1) queueFamilies.graphicsFamily = i;

			if (queueFamilies.presentFamily == -1 && window.CanPresent(*this, i))
			{
				queueFamilies.presentFamily = i;
				queueFamilies.graphicsFamily = i;
			}
		}

		if (queueFamilies.computeFamily == -1 && queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			if (queueFamilies.presentFamily != i && queueFamilies.presentFamily != i) queueFamilies.computeFamily = i;
		}
	}

	if (queueFamilies.graphicsFamily == -1 || queueFamilies.presentFamily == -1) throw (std::runtime_error("Failed to find required queue families"));
}

void Device::RetrieveQueues()
{
	if (!logicalDevice) throw (std::runtime_error("Logical device does not exist"));
	if (queueFamilies.graphicsFamily == -1 ||queueFamilies.presentFamily == -1) throw (std::runtime_error("Queue families not complete"));

	vkGetDeviceQueue(logicalDevice, queueFamilies.graphicsFamily, 0, &queueFamilies.graphicsQueue);
	vkGetDeviceQueue(logicalDevice, queueFamilies.presentFamily, 0, &queueFamilies.presentQueue);
}

void Device::Destroy()
{
	if (logicalDevice)
	{
		vkDestroyDevice(logicalDevice, nullptr);
		logicalDevice = nullptr;
		physicalDevice = nullptr;
		queueFamilies = QueueFamilies{};
	}
}

const bool Device::Created() const
{
	return (physicalDevice != nullptr && logicalDevice != nullptr);
}

VkPhysicalDevice& Device::GetPhysicalDevice()
{
	if (!physicalDevice) throw (std::runtime_error("Physical device requested but not yet created"));

	return (physicalDevice);
}

VkDevice& Device::GetLogicalDevice()
{
	if (!logicalDevice) throw (std::runtime_error("Logical device requested but not yet created"));

	return (logicalDevice);
}

uint32_t Device::GetQueueIndex(QueueType type)
{
	int index = -1;

	switch (type)
	{
		case QueueType::Graphics: index = queueFamilies.graphicsFamily; break;
		case QueueType::Compute: index = queueFamilies.computeFamily; break;
		case QueueType::Present: index = queueFamilies.presentFamily; break;
	}

	if (index == -1) throw (std::runtime_error("Queue index does not exist"));

	return (CUI(index));
}

VkQueue Device::GetQueue(uint32_t index)
{
	if (queueFamilies.graphicsFamily >= 0 && static_cast<uint32_t>(queueFamilies.graphicsFamily) == index)
		return (queueFamilies.graphicsQueue);
	else if (queueFamilies.computeFamily >= 0 && static_cast<uint32_t>(queueFamilies.computeFamily) == index)
		return (queueFamilies.computeQueue);
	else if (queueFamilies.presentFamily >= 0 && static_cast<uint32_t>(queueFamilies.presentFamily) == index)
		return (queueFamilies.presentQueue);
	else
		throw (std::runtime_error("There is no queue with the given index"));

	return (nullptr);
}

uint32_t Device::FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties)
{
	if (!physicalDevice) throw (std::runtime_error("Physical device does not exist"));

	VkPhysicalDeviceMemoryProperties memoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((filter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return (i);
		}
	}

	throw (std::runtime_error("Failed to find a suitable memory type"));
}

std::vector<DeviceInfo> Device::GetAvailableDevices()
{
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(Graphics::GetInstance(), &deviceCount, nullptr);
	if (deviceCount <= 0) throw (std::runtime_error("Failed to find a GPU with Vulkan support"));

	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(Graphics::GetInstance(), &deviceCount, physicalDevices.data());

	std::vector<DeviceInfo> availableDevices(deviceCount);
	for (int i = 0; i < deviceCount; i++)
	{
		availableDevices[i].physicalDevice = physicalDevices[i];
		vkGetPhysicalDeviceProperties(availableDevices[i].physicalDevice, &availableDevices[i].deviceProperties);
		vkGetPhysicalDeviceFeatures(availableDevices[i].physicalDevice, &availableDevices[i].deviceFeatures);
		availableDevices[i].type = static_cast<DeviceType>(availableDevices[i].deviceProperties.deviceType);
	}

	return (availableDevices);
}

DeviceInfo Device::GetBestDevice(DeviceConfig& config)
{
	std::vector<DeviceInfo> availableDevices = GetAvailableDevices();
	bool any = config.type == DeviceType::Best;
	int best = -1;
	
	for (int i = 0; i < availableDevices.size(); i++)
	{
		if (any && DeviceTypePriority(availableDevices[i].type) < DeviceTypePriority(config.type)) continue;
		else if (!any && availableDevices[i].type != config.type) continue;
		else if (config.tesselation && !availableDevices[i].deviceFeatures.tessellationShader) continue;
		else if (config.anisotropic && !availableDevices[i].deviceFeatures.samplerAnisotropy) continue;
		else if (config.shaderDouble && !availableDevices[i].deviceFeatures.shaderFloat64) continue;

		best = i;
		config.type = availableDevices[i].type;
	}

	if (best == -1) throw (std::runtime_error("Failed to find the best suitable device"));

	return (availableDevices[best]);
}

int Device::DeviceTypePriority(DeviceType type)
{
	switch (type)
	{
		case DeviceType::CPU: return (1);
		case DeviceType::Virtual: return (2);
		case DeviceType::Integrated: return (3);
		case DeviceType::Discrete: return (4);
		default: return (-1);
	}

	return (-1);
}

std::ostream& operator<<(std::ostream& out, Device& device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device.GetPhysicalDevice(), &deviceProperties);

	out << std::endl;
	out << VAR_VAL(deviceProperties.deviceName) << std::endl;
	out << VAR_VAL(deviceProperties.vendorID) << std::endl;
	out << ENUM_VAL(deviceProperties.deviceType) << std::endl;
	out << VAR_VAL(deviceProperties.driverVersion) << std::endl;
	out << VAR_VAL(deviceProperties.apiVersion) << std::endl;

	return (out);
}