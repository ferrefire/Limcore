#pragma once

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "camera.hpp"

#include <functional>
#include <filesystem>

/**
 * @file manager.hpp
 * @brief Application lifecycle manager for windowing, devices, and rendering.
 *
 * @details
 * Provides a static manager class that encapsulates initialization and destruction
 * of GLFW and Vulkan resources, including the main Window, Device, Swapchain, and Camera.
 * Supports a run loop with per-frame callbacks and hooks for startup, frame, end, and resize events.
 */

/** @brief Configuration options for the application manager. */
struct ManagerConfig
{
	std::filesystem::path executeablePath;

	bool fullscreen = false; /**< @brief Whether to create the window in fullscreen mode. */
	bool integrated = false; /**< @brief Prefer integrated GPU over discrete GPU when selecting a device. */
};

/**
 * @brief Central manager for application lifecycle and rendering systems.
 *
 * @details
 * Handles initialization and shutdown of GLFW and Vulkan, manages the primary
 * rendering objects (window, device, swapchain, camera), and drives the main loop.
 * 
 * Typical usage:
 * - Configure via @ref ParseArguments() or default settings.
 * - Call @ref Create() to initialize systems.
 * - Register callbacks for @ref Start(), @ref Frame(), @ref Resize(), and @ref Destroy().
 * - Run the application with @ref Run().
 */
class Manager
{
	private:
		static ManagerConfig config;

		static Window window;
		static Device device;
		static Swapchain swapchain;
		static Camera camera;

		static bool stopping;
		static bool resizing;

		static std::vector<std::function<void()>> startCalls;
		static std::vector<std::function<void()>> frameCalls;
		static std::vector<std::function<void()>> endCalls;
		static std::vector<std::function<void()>> resizeCalls;

		static void CreateGLFW();
		static void CreateVulkan();

		static void DestroyGLFW();
		static void DestroyVulkan();

		static void ResizeCallback(GLFWwindow* data, int width, int height);

	public:
		Manager();
		~Manager();

		/** @brief Initializes GLFW, Vulkan, and core rendering objects. */
		static void Create();
		
		/** @brief Shuts down the application and releases resources. */
		static void Destroy();

		/**
		 * @brief Gets the main window object.
		 * @return Reference to the Window.
		 */
		static Window& GetWindow();

		/**
		 * @brief Gets the current Device.
		 * @return Reference to the Device.
		 */
		static Device& GetDevice();

		/**
		 * @brief Gets the swapchain.
		 * @return Reference to the Swapchain.
		 */
		static Swapchain& GetSwapchain();

		/**
		 * @brief Gets the main camera.
		 * @return Reference to the Camera.
		 */
		static Camera& GetCamera();

		/** @brief This will call all registered start functions and begin the main application loop. */
		static void Run();
		
		static void Start();
		static void Frame();
		static void Resize(bool force = true);
		
		/**
		 * @brief Parses command-line arguments for configuration.
		 * @param arguments Argument list.
		 * @param count Number of arguments.
		 */
		static void ParseArguments(char **arguments, const int& count);

		/**
		 * @brief Checks if the window should be closed.
		 * @return True if the window should be closed.
		 */
		static bool ShouldClose();

		/**
		 * @brief Registers a callback to run once at startup.
		 * @param call Function to register.
		 */
		static void RegisterStartCall(std::function<void()> call);

		/**
		 * @brief Registers a callback to run each frame.
		 * @param call Function to register.
		 */
		static void RegisterFrameCall(std::function<void()> call);

		/**
		 * @brief Registers a callback to run at shutdown.
		 * @param call Function to register.
		 */
		static void RegisterEndCall(std::function<void()> call);


		/**
		 * @brief Registers a callback to run when the window is resized.
		 * @param call Function to register.
		 */
		static void RegisterResizeCall(std::function<void()> call);

		template <class T>
		static void RegisterResizeCall(T* object, void (T::*call)()) { RegisterResizeCall(std::bind_front(call, object)); }

		static const std::filesystem::path& GetExecuteablePath();
};