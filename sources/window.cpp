#include "window.hpp"

#include "graphics.hpp"
#include "utilities.hpp"
#include "printer.hpp"
#include "input.hpp"

#include <stdexcept>
#include <iostream>

Window::Window()
{

}

Window::~Window()
{
	DestroyFrame();
	DestroySurface();
}

void Window::SetConfig(const WindowConfig& windowConfig)
{
	config = windowConfig;
}

void Window::CreateFrame()
{
	if (data) throw (std::runtime_error("Window already exists"));

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	width = (config.fullscreen ? mode->width : mode->width / 2);
	height = (config.fullscreen ? mode->height : mode->height / 2);

	if (config.fullscreen)
	{
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	}

	//data = glfwCreateWindow(width, height, "limcore", config.fullscreen ? monitor : nullptr, nullptr);
	data = glfwCreateWindow(width, height, "limcore", nullptr, nullptr);

	if (!data) throw (std::runtime_error("Failed to create window"));

	if (config.fullscreen) glfwSetWindowMonitor(data, nullptr, 0, 0, width, height, mode->refreshRate);
	glfwSetInputMode(data, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(data, Input::MouseCallback);
	glfwSetScrollCallback(data, Input::ScrollCallback);
}

void Window::CreateSurface(Device& device)
{
	if (!data) throw (std::runtime_error("Window does not exist yet"));

	if (glfwCreateWindowSurface(Graphics::GetInstance(), data, nullptr, &surface) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create window surface"));

	//if (!CanPresent(device)) throw (std::runtime_error("Window surface can not be presented to"));

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.GetPhysicalDevice(), surface, &config.capabilities);
	SelectFormat(device);
	SelectPresentMode(device);

	int width, height;
	glfwGetFramebufferSize(data, &width, &height);
	config.extent.width = width;
	config.extent.height = height;
}

void Window::DestroyFrame()
{
	if (data)
	{
		glfwDestroyWindow(data);
		data = nullptr;
	}
}

void Window::DestroySurface()
{
	if (surface)
	{
		vkDestroySurfaceKHR(Graphics::GetInstance(), surface, nullptr);
		surface = nullptr;
	}

	config = WindowConfig{};
}

GLFWwindow* Window::GetData() const
{
	if (!data) throw (std::runtime_error("Window data requested but not yet created"));

	return (data);
}

const VkSurfaceKHR& Window::GetSurface() const
{
	if (!surface) throw (std::runtime_error("Window surface requested but not yet created"));

	return (surface);
}

const WindowConfig& Window::GetConfig() const
{
	return (config);
}

bool Window::CanPresent(Device& device, int presentQueueIndex = -1)
{
	if (!surface) throw (std::runtime_error("Window surface does not exist yet"));

	uint32_t queueIndex = (presentQueueIndex == -1 ? device.GetQueueIndex(QueueType::Present) : CUI(presentQueueIndex));
	VkBool32 canPresent = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(device.GetPhysicalDevice(), queueIndex, surface, &canPresent);

	return (canPresent);
}

void Window::SelectFormat(Device& device)
{
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.GetPhysicalDevice(), surface, &formatCount, nullptr);
	
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device.GetPhysicalDevice(), surface, &formatCount, formats.data());

	bool correctFormatFound = false;
	for (VkSurfaceFormatKHR availableFormat : formats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			config.format = availableFormat;
			correctFormatFound = true;
		}
	}

	if (!correctFormatFound) throw (std::runtime_error("Failed to find correct surface format"));
}

void Window::SelectPresentMode(Device& device)
{
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.GetPhysicalDevice(), surface, &presentModeCount, nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device.GetPhysicalDevice(), surface, &presentModeCount, presentModes.data());

	bool correctPresentModeFound = false;
	bool defaultPresentModeFound = false;
	for (VkPresentModeKHR availablePresentMode : presentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
		{
			defaultPresentModeFound = true;
		}
		if (availablePresentMode == config.presentMode)
		{
			correctPresentModeFound = true;
		}
	}

	if (!correctPresentModeFound && !defaultPresentModeFound) throw (std::runtime_error("Failed to find correct surface present mode"));

	if (!correctPresentModeFound) config.presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

std::ostream& operator<<(std::ostream& out, Window& window)
{
	WindowConfig config = window.GetConfig();

	out << std::endl;
	out << VAR_VAL(config.extent.width) << std::endl;
	out << VAR_VAL(config.extent.height) << std::endl;
	out << ENUM_VAL(config.format.format) << std::endl;
	out << ENUM_VAL(config.format.colorSpace) << std::endl;
	out << ENUM_VAL(config.presentMode) << std::endl;
	out << VAR_VAL(config.capabilities.minImageCount) << std::endl;
	out << VAR_VAL(config.capabilities.maxImageCount) << std::endl;
	
	return (out);
}