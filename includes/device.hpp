#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

enum class QueueType { Graphics, Compute, Present };

enum class DeviceType 
{ 
	Integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
	Discrete = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
	Virtual = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
	CPU = VK_PHYSICAL_DEVICE_TYPE_CPU,
	Best = -1,
};

struct DeviceConfig
{
	DeviceType type = DeviceType::Best;
	bool tesselation = false;
	bool anisotropic = false;
	bool shaderDouble = true;
};

struct QueueFamilies
{
	int graphicsFamily = -1;
	int computeFamily = -1;
	int presentFamily = -1;

	VkQueue graphicsQueue = nullptr;
	VkQueue computeQueue = nullptr;
	VkQueue presentQueue = nullptr;
};

struct DeviceInfo
{
	VkPhysicalDevice physicalDevice = nullptr;
	VkPhysicalDeviceProperties deviceProperties{};
	VkPhysicalDeviceFeatures deviceFeatures{};
	DeviceType type = DeviceType::Best;
};

class Device
{
	private:
		DeviceConfig config{};

		VkPhysicalDevice physicalDevice = nullptr;
		VkDevice logicalDevice = nullptr;
		QueueFamilies queueFamilies{};
		
	public:
		Device();
		~Device();

		void SetConfig(const DeviceConfig& deviceConfig);
		void CreatePhysical();
		void CreateLogical();
		void SelectQueues();
		void RetrieveQueues();

		void Destroy();

		const bool Created() const;
		VkPhysicalDevice& GetPhysicalDevice();
		VkDevice& GetLogicalDevice();
		uint32_t GetQueueIndex(QueueType type);
		VkQueue GetQueue(uint32_t index);
		uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);

		static std::vector<DeviceInfo> GetAvailableDevices();
		static DeviceInfo GetBestDevice(DeviceConfig& config);
		static int DeviceTypePriority(DeviceType type);
};

std::ostream& operator<<(std::ostream& out, Device& device);