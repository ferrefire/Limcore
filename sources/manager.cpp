#include "manager.hpp"

#include "graphics.hpp"
#include "renderer.hpp"
#include "utilities.hpp"
#include "input.hpp"
#include "time.hpp"
#include "descriptor.hpp"
#include "command.hpp"

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
		std::cerr << "Error occured during manager creation: " << e.what() << '\n';

		Manager::Destroy();
		exit(EXIT_FAILURE);
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
	if (config.integrated) config.deviceConfig.type = DeviceType::Integrated;
	if (config.wireframe) config.deviceConfig.wireframeMode = true;
	device.SetConfig(config.deviceConfig);

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

	if (config.framesInFlight > swapchain.GetFrameCount()) config.framesInFlight = swapchain.GetFrameCount();
	std::cout << "Frames in flight: " << config.framesInFlight << std::endl << std::endl;

	Command::CreatePools();

	Descriptor::CreatePools();

	Renderer::Create(config.framesInFlight, &device, &swapchain);

	CameraConfig cameraConfig{};
	cameraConfig.width = window.GetConfig().extent.width;
	cameraConfig.height = window.GetConfig().extent.height;
	camera.Create(cameraConfig);
}

void Manager::Destroy()
{
	try
	{
		if (device.Created() && vkDeviceWaitIdle(device.GetLogicalDevice()) != VK_SUCCESS)
			throw (std::runtime_error("Failed to wait for device to be idle"));

		for (std::function<void()> call : endCalls) { call(); }

		DestroyVulkan();
		DestroyGLFW();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error occured during manager destruction: " << e.what() << '\n';
	}
}

void Manager::DestroyGLFW()
{
	window.DestroyFrame();
	glfwTerminate();
}

void Manager::DestroyVulkan()
{
	if (device.Created())
	{
		swapchain.Destroy();
		Renderer::Destroy();
		Descriptor::DestroyPools();
		Command::DestroyPools();
		window.DestroySurface();
		device.Destroy();
	}

	Graphics::DestroyInstance();
}

void Manager::ResizeCallback(GLFWwindow* data, int width, int height)
{
	resizing = true;
}

ManagerConfig& Manager::GetConfig()
{
	return (config);
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

Camera& Manager::GetCamera()
{
	return (camera);
}

void Manager::Run()
{
	try { Start(); }
	catch(const std::exception& e) { std::cerr << "Error occured during application start: " << e.what() << '\n'; }
	
	try { while (!ShouldClose()) { Frame(); } }
	catch(const std::exception& e) { std::cerr << "Error occured during main loop: " << e.what() << '\n'; }
}

void Manager::Start()
{
	for (std::function<void()> call : startCalls) { call(); }
}

void Manager::Frame()
{
	glfwPollEvents();

	Renderer::WaitForFrame();

	if (resizing) Resize();

	Time::Frame();
	Input::Frame();

	for (std::function<void()> call : frameCalls) { call(); }

	Renderer::Frame();

	if (Input::GetKey(GLFW_KEY_ESCAPE).pressed) stopping = true;
}

void Manager::Resize(bool force)
{
	if (!force)
	{
		resizing = true;
		return;
	}

	resizing = false;

	window.Resize(device);
	camera.Resize(window.GetConfig().extent.width, window.GetConfig().extent.height);
	
	vkDeviceWaitIdle(device.GetLogicalDevice());
	swapchain.Destroy();
	swapchain.Create(&window, &device);

	Renderer::Resize();

	for (std::function<void()> call : resizeCalls) { call(); }
}

void Manager::ParseArguments(char** arguments, const int& count)
{
	if (count <= 0) return;

	std::filesystem::path argumentPath = arguments[0];
	executeablePath = std::filesystem::absolute(executeablePath);

	for (int i = 1; i < count; i++)
	{
		if (std::string(arguments[i]) == "fs") config.fullscreen = true;
		else if (std::string(arguments[i]) == "ig") config.integrated = true;
	}
}

bool Manager::ShouldClose()
{
	return (stopping || glfwWindowShouldClose(window.GetData()));
}

void Manager::RegisterStartCall(std::function<void()> call)
{
	startCalls.push_back(call);
}

void Manager::RegisterFrameCall(std::function<void()> call)
{
	frameCalls.push_back(call);
}

void Manager::RegisterEndCall(std::function<void()> call)
{
	endCalls.push_back(call);
}

void Manager::RegisterResizeCall(std::function<void()> call)
{
	resizeCalls.push_back(call);
}

const std::filesystem::path& Manager::GetExecuteablePath()
{
	return (executeablePath);
}

ManagerConfig Manager::config{};
std::filesystem::path Manager::executeablePath = std::filesystem::current_path();

Window Manager::window;
Device Manager::device;
Swapchain Manager::swapchain;
Camera Manager::camera;

bool Manager::stopping = false;
bool Manager::resizing = false;

std::vector<std::function<void()>> Manager::startCalls;
std::vector<std::function<void()>> Manager::frameCalls;
std::vector<std::function<void()>> Manager::endCalls;
std::vector<std::function<void()>> Manager::resizeCalls;