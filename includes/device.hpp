#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

/**
 * @file device.hpp
 * @brief Vulkan device management and utilities.
 *
 * @details
 * Provides configuration, selection, and management of Vulkan physical
 * and logical devices, including queue families and capabilities.
 * Includes helpers for enumerating available devices and selecting
 * the most suitable GPU for rendering.
 */

/** @brief An enum for different types of Device queues. */
enum class QueueType { Graphics, Compute, Present };

/** @brief An enum for different types of Devices. */
enum class DeviceType 
{ 
	Integrated = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
	Discrete = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
	Virtual = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
	CPU = VK_PHYSICAL_DEVICE_TYPE_CPU,
	Best = -1,
};

/** @brief Describes a Device configuration. */
struct DeviceConfig
{
	DeviceType type = DeviceType::Best; /**< @brief Desired device type (or automatic). */
	bool tesselation = false;
	bool anisotropic = false;
	bool shaderDouble = false;
};

/** @brief Contains information about different queue families. */
struct QueueFamilies
{
	int graphicsFamily = -1;
	int computeFamily = -1;
	int presentFamily = -1;

	VkQueue graphicsQueue = nullptr;
	VkQueue computeQueue = nullptr;
	VkQueue presentQueue = nullptr;
};

/** @brief Information about a physical device.  */
struct DeviceInfo
{
	VkPhysicalDevice physicalDevice = nullptr;
	VkPhysicalDeviceProperties deviceProperties{};
	VkPhysicalDeviceFeatures deviceFeatures{};
	DeviceType type = DeviceType::Best;
};

/**
 * @brief Vulkan device manager.
 *
 * @details
 * Encapsulates physical and logical device creation, configuration,
 * and queue family retrieval. Provides utilities for selecting
 * the most suitable GPU and accessing its queues and memory types.
 */
class Device
{
	private:
		DeviceConfig config{};

		VkPhysicalDevice physicalDevice = nullptr;
		VkDevice logicalDevice = nullptr;
		QueueFamilies queueFamilies{};
		
	public:
		/** @brief Constructs an empty Device. */
		Device();

		/** @brief Destroys the logical device if created. */
		~Device();

		void SetConfig(const DeviceConfig& deviceConfig);
		void CreatePhysical();
		void CreateLogical();
		void SelectQueues();
		void RetrieveQueues();

		void Destroy();

		/**
		 * @brief Checks if a logical device has been created.
		 * @return True if the logical device exists.
		 */
		const bool Created() const;
		VkPhysicalDevice& GetPhysicalDevice();
		const VkPhysicalDevice& GetPhysicalDevice() const;
		VkDevice& GetLogicalDevice();
		uint32_t GetQueueIndex(QueueType type);
		VkQueue GetQueue(uint32_t index);
		uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);

		/**
		 * @brief Gets a list of all available Devices.
		 * @return A vector of DeviceInfo structs.
		 */
		static std::vector<DeviceInfo> GetAvailableDevices();

		/**
		 * @brief Selects the best device given a configuration.
		 * @param config Configuration preferences.
		 * @return Information about the selected device.
		 */
		static DeviceInfo GetBestDevice(DeviceConfig& config);
		static int DeviceTypePriority(DeviceType type);

		/** @brief Prints information about all available devices to stdout. */
		static void PrintAllDevices();
};

std::ostream& operator<<(std::ostream& out, const VkPhysicalDeviceProperties& deviceProperties);
std::ostream& operator<<(std::ostream& out, const Device& device);