#include "manager.hpp"

#include "graphics.hpp"
#include "renderer.hpp"
#include "descriptor.hpp"
#include "utilities.hpp"
#include "input.hpp"
#include "time.hpp"

#include "pipeline.hpp"
#include "mesh.hpp"
#include "buffer.hpp"
#include "pass.hpp"
#include "bitmask.hpp"
#include "command.hpp"
#include "vertex.hpp"
#include "shape.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <utility>

Manager::Manager()
{

}

Manager::~Manager()
{
	Destroy();
}

void Manager::Create()
{
	try
	{
		CreateGLFW();
		CreateVulkan();
	}
	catch(const std::exception& e)
	{
		throw (std::runtime_error(e.what()));
	}
}

void Manager::CreateGLFW()
{
	if (!glfwInit()) throw (std::runtime_error("Failed to initiate GLFW"));

	WindowConfig windowConfig{};
	windowConfig.fullscreen = config.fullscreen;
	window.SetConfig(windowConfig);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window.CreateFrame();
}

void Manager::CreateVulkan()
{
	DeviceConfig deviceConfig{};
	if (config.integrated) deviceConfig.type = DeviceType::Integrated;
	device.SetConfig(deviceConfig);

	Graphics::CreateInstance();
	device.CreatePhysical();
	window.CreateSurface(device);
	std::cout << "Window created: " << window << std::endl;
	device.SelectQueues();
	device.CreateLogical();
	device.RetrieveQueues();
	std::cout << "Device created: " << device << std::endl;
	swapchain.Create(&window, &device);
	std::cout << "Swapchain created: " << swapchain << std::endl;
	Renderer::Create(swapchain.GetFrameCount(), &device, &swapchain);
}

void Manager::Destroy()
{
	if (device.Created() && vkDeviceWaitIdle(device.GetLogicalDevice()) != VK_SUCCESS)
		throw (std::runtime_error("Failed to wait for device to be idle"));

	DestroyGLFW();
	DestroyVulkan();

	exit(EXIT_SUCCESS);
}

void Manager::DestroyGLFW()
{
	window.DestroyFrame();
	glfwTerminate();
}

void Manager::DestroyVulkan()
{
	for (std::function<void()> call : endCalls) { call(); }

	if (device.Created())
	{
		swapchain.Destroy();
		Renderer::Destroy();
		window.DestroySurface();
		device.Destroy();
	}

	Graphics::DestroyInstance();
}

Window& Manager::GetWindow()
{
	return (window);
}

Device& Manager::GetDevice()
{
	return (device);
}

Swapchain& Manager::GetSwapchain()
{
	return (swapchain);
}

void Manager::Run()
{
	Start();

	while (!ShouldClose()) { Frame(); }
	
	Destroy();
}

void Manager::Start()
{
	for (std::function<void()> call : startCalls) { call(); }
}

void Manager::Frame()
{
	if (Input::GetKey(GLFW_KEY_ESCAPE).pressed) Destroy();

	glfwPollEvents();

	Time::Frame();
	Input::Frame();

	Renderer::Frame();
}

void Manager::ParseArguments(char** arguments, const int& count)
{
	for (int i = 1; i < count; i++)
	{
		if (std::string(arguments[i]) == "fs") config.fullscreen = true;
		else if (std::string(arguments[i]) == "ig") config.integrated = true;
	}
}

bool Manager::ShouldClose()
{
	return (glfwWindowShouldClose(window.GetData()));
}

void Manager::RegisterStartCall(std::function<void()> call)
{
	startCalls.push_back(call);
}

void Manager::RegisterEndCall(std::function<void()> call)
{
	endCalls.push_back(call);
}

ManagerConfig Manager::config{};

Window Manager::window;
Device Manager::device;
Swapchain Manager::swapchain;

std::vector<std::function<void()>> Manager::startCalls;
std::vector<std::function<void()>> Manager::endCalls;