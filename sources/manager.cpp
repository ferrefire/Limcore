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

/*struct UniformData
{
	dpoint2D center;
	double zoom;
	float aspect;
	int maxIterations;
};

void Test(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	static bool start = true;
	static Mesh<Position | Coordinate, VK_INDEX_TYPE_UINT16> mesh;
	static Pass pass;
	static Pipeline pipeline;
	static UniformData uniformData;
	static Buffer buffer;
	static Descriptor descriptor;

	if (start)
	{
		uniformData.center = dpoint2D({-0.5, 0.0});
		uniformData.zoom = 1.0;
		uniformData.aspect = float(Manager::GetWindow().GetConfig().extent.width) / float(Manager::GetWindow().GetConfig().extent.height);
		uniformData.maxIterations = 30;

		mesh.SetShape(Shape<Position | Coordinate, VK_INDEX_TYPE_UINT16>(ShapeType::Quad));

		mesh.Create(&Manager::GetDevice());

		PassConfig passConfig = Pass::DefaultConfig();
		pass.Create(passConfig, &Manager::GetDevice());

		BufferConfig bufferConfig{};
		bufferConfig.mapped = true;
		bufferConfig.size = sizeof(uniformData);
		buffer.Create(bufferConfig, &Manager::GetDevice(), &uniformData);

		std::vector<DescriptorConfig> descriptorConfigs(1);
		descriptorConfigs[0].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor.Create(descriptorConfigs, &Manager::GetDevice());
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer.GetBuffer();
		bufferInfo.range = sizeof(uniformData);
		descriptor.Update(0, &bufferInfo, nullptr);

		PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
		pipelineConfig.shader = "screen";
		pipelineConfig.vertexInfo = mesh.GetVertexInfo();
		pipelineConfig.renderpass = pass.GetRenderpass();
		pipelineConfig.rasterization.cullMode = VK_CULL_MODE_NONE;
		pipelineConfig.descriptorLayouts = {descriptor.GetLayout()};
		pipeline.Create(pipelineConfig, &Manager::GetDevice());

		Renderer::AddPass(&pass);

		start = false;
	}
	else if (commandBuffer)
	{
		if (Input::GetKey(GLFW_KEY_W).down) uniformData.center.y() -= Time::deltaTime / uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_S).down) uniformData.center.y() += Time::deltaTime / uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_A).down) uniformData.center.x() -= Time::deltaTime / uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_D).down) uniformData.center.x() += Time::deltaTime / uniformData.zoom;

		if (Input::GetKey(GLFW_KEY_UP).down) uniformData.zoom += Time::deltaTime * uniformData.zoom;
		if (Input::GetKey(GLFW_KEY_DOWN).down) uniformData.zoom -= Time::deltaTime * uniformData.zoom;

		if (Input::GetKey(GLFW_KEY_RIGHT).pressed || (Input::GetKey(GLFW_KEY_RIGHT).holding && Time::newTick)) 
			uniformData.maxIterations += 2;
		if (Input::GetKey(GLFW_KEY_LEFT).pressed || (Input::GetKey(GLFW_KEY_LEFT).holding && Time::newTick)) 
			uniformData.maxIterations -= 2;

		buffer.Update(&uniformData, sizeof(uniformData));

		pipeline.Bind(commandBuffer);
		descriptor.Bind(commandBuffer, pipeline.GetLayout());
		mesh.Bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, CUI(mesh.GetIndices().size()), 1, 0, 0, 0);
	}
	else
	{
		mesh.Destroy();
		buffer.Destroy();
		descriptor.Destroy();
		pass.Destroy();
		pipeline.Destroy();

		start = true;
	}
}*/

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
	if (device.Created())
	{
		//Test(nullptr, 0);
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
	//Test(nullptr, 0);
	//Renderer::RegisterCall(Test);
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

ManagerConfig Manager::config{};

Window Manager::window;
Device Manager::device;
Swapchain Manager::swapchain;