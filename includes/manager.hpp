#pragma once

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"

#include <functional>

struct ManagerConfig
{
	bool fullscreen = false;
	bool integrated = false;
};

class Manager
{
	private:
		static ManagerConfig config;

		static Window window;
		static Device device;
		static Swapchain swapchain;

		static std::vector<std::function<void()>> startCalls;
		static std::vector<std::function<void()>> endCalls;

		static void CreateGLFW();
		static void CreateVulkan();

		static void DestroyGLFW();
		static void DestroyVulkan();

	public:
		Manager();
		~Manager();

		static void Create();
		
		static void Destroy();

		static Window& GetWindow();
		static Device& GetDevice();
		static Swapchain& GetSwapchain();

		static void Run();
		static void Start();
		static void Frame();
		
		static void ParseArguments(char **arguments, const int& count);
		static bool ShouldClose();

		static void RegisterStartCall(std::function<void()> call);
		static void RegisterEndCall(std::function<void()> call);
};