#include "graphics.hpp"

#include "manager.hpp"
#include "printer.hpp"
#include "utilities.hpp"

#include <stdexcept>
#include <string>
#include <iostream>

Graphics::Graphics()
{

}

Graphics::~Graphics()
{
	DestroyInstance();
}

void Graphics::CreateInstance()
{
	if (instance) throw (std::runtime_error("Vulkan instance already exists"));

	if (Manager::GetConfig().useValidationLayers && HasValidationLayers()) validationLayersEnabled = true;
	std::cout << "Vulkan validation layers: " << (validationLayersEnabled ? "enabled" : "disabled") << std::endl << std::endl;

	uint32_t instanceVersion = VK_API_VERSION_1_0;
	PFN_vkEnumerateInstanceVersion pfnEnumerateInstanceVersion =
	    reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
	        vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

	if (pfnEnumerateInstanceVersion) 
	{
	    pfnEnumerateInstanceVersion(&instanceVersion);
	}

	printf("Loader/Runtime instance version: %u.%u.%u\n",
	       VK_VERSION_MAJOR(instanceVersion),
	       VK_VERSION_MINOR(instanceVersion),
	       VK_VERSION_PATCH(instanceVersion));


	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "limcore";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "None";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (glfwExtensions == nullptr) 
		throw (std::runtime_error("Vulkan is either not available on this machine or it does not support window creation"));

	std::vector<const char*> extensions(glfwExtensionCount);
	for (int i = 0; i < glfwExtensionCount; i++) { extensions[i] = glfwExtensions[i]; }

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.enabledExtensionCount = CUI(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = validationLayersEnabled ? CUI(validationLayers.size()) : 0;
	createInfo.ppEnabledLayerNames = validationLayersEnabled ? validationLayers.data() : nullptr;
	createInfo.pApplicationInfo = &appInfo;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create Vulkan instance"));

	std::cout << "Vulkan instance created" << std::endl << std::endl;
}

void Graphics::DestroyInstance()
{
	if (instance)
	{
		vkDestroyInstance(instance, nullptr);
		instance = nullptr;
	}
}

const VkInstance& Graphics::GetInstance()
{
	if (!instance) throw (std::runtime_error("Instance requested but not yet created"));

	return (instance);
}

bool Graphics::HasValidationLayers()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layer : validationLayers)
	{
		bool layerFound = false;

		for (const VkLayerProperties &layerProperties : availableLayers)
		{
			if (std::string(layerProperties.layerName).compare(layer) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return (false);
	}

	return (true);
}

VkInstance Graphics::instance = nullptr;

bool Graphics::validationLayersEnabled = false;
std::vector<const char*> Graphics::validationLayers = {"VK_LAYER_KHRONOS_validation"};