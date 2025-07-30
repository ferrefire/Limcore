#include "swapchain.hpp"

#include "manager.hpp"
#include "image.hpp"
#include "printer.hpp"
#include "utilities.hpp"

#include <stdexcept>
#include <iostream>

Swapchain::Swapchain()
{

}

Swapchain::~Swapchain()
{
	Destroy();
}

void Swapchain::Create(Window* swapchainWindow = nullptr, Device* swapchainDevice = nullptr)
{
	window = swapchainWindow;
	device = swapchainDevice;

	if (!window) window = &Manager::GetWindow();
	if (!device) device = &Manager::GetDevice();

	CreateSwapchain();
	RetrieveImages();
	CreateViews();
}

void Swapchain::CreateSwapchain()
{
	if (swapchain) throw (std::runtime_error("Swapchain already exists"));
	if (!window) throw (std::runtime_error("Swapchain has no window"));
	if (!device) throw (std::runtime_error("Swapchain has no device"));

	WindowConfig windowConfig = window->GetConfig();

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = window->GetSurface();
	createInfo.minImageCount = windowConfig.capabilities.minImageCount;
	createInfo.imageFormat = windowConfig.format.format;
	createInfo.imageColorSpace = windowConfig.format.colorSpace;
	createInfo.imageExtent = windowConfig.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = windowConfig.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = windowConfig.presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = nullptr;

	if (vkCreateSwapchainKHR(device->GetLogicalDevice(), &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create swapchain"));
}

void Swapchain::RetrieveImages()
{
	if (images.size() != 0) throw (std::runtime_error("Swapchain images already exist"));
	if (!swapchain) throw (std::runtime_error("Swapchain does not exist"));
	if (!device) throw (std::runtime_error("Swapchain has no device"));

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(device->GetLogicalDevice(), swapchain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device->GetLogicalDevice(), swapchain, &imageCount, images.data());

	if (images.size() == 0) throw (std::runtime_error("Failed to retrieve swapchain images"));

	frameCount = CUI(images.size());
}

void Swapchain::CreateViews()
{
	if (views.size() != 0) throw (std::runtime_error("Swapchain views already exist"));
	if (images.size() == 0) throw (std::runtime_error("Swapchain images do not exist"));
	if (!swapchain) throw (std::runtime_error("Swapchain does not exist"));
	if (!window) throw (std::runtime_error("Swapchain has no window"));
	if (!device) throw (std::runtime_error("Swapchain has no device"));

	views.resize(images.size());
	for (int i = 0; i < images.size(); i++)
	{
		ImageViewConfig viewConfig = Image::DefaultViewConfig();
		viewConfig.format = window->GetConfig().format.format;

		Image::CreateView(views[i], images[i], viewConfig, device);
	}
}

void Swapchain::Destroy()
{
	if (!device) return;

	for (VkImageView& view : views)
	{
		vkDestroyImageView(device->GetLogicalDevice(), view, nullptr);
	}
	views.clear();

	if (swapchain)
	{
		vkDestroySwapchainKHR(device->GetLogicalDevice(), swapchain, nullptr);
		swapchain = nullptr;
		images.clear();
	}
}

const VkSwapchainKHR& Swapchain::GetSwapchain() const
{
	if (!swapchain) throw (std::runtime_error("Swapchain requested but does not exist"));

	return (swapchain);
}

const uint32_t Swapchain::GetFrameCount() const
{
	return (frameCount);
}

const std::vector<VkImage>& Swapchain::GetImages()
{
	if (images.size() == 0) throw (std::runtime_error("Swapchain images requested but don't exist"));

	return (images);
}

const std::vector<VkImageView>& Swapchain::GetViews()
{
	if (views.size() == 0) throw (std::runtime_error("Swapchain views requested but don't exist"));

	return (views);
}

std::ostream& operator<<(std::ostream& out, Swapchain& swapchain)
{
	out << std::endl;
	out << VAR_VAL(swapchain.GetViews().size()) << std::endl;

	return (out);
}