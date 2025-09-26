#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <vector>
#include <functional>

/**
 * @file input.hpp
 * @brief Input handling system for keyboard, mouse, and scroll events.
 *
 * @details
 * Provides a static input manager that tracks key states, mouse position,
 * and scroll input. Supports per-frame updates, key queries, and registration
 * of callback functions for mouse movement and scrolling.
 */

/** @brief Contains information about the state of a key. */
struct KeyInfo
{
	bool down = false; /**< @brief True while the key is currently held down. */
	bool pressed = false; /**< @brief True on the frame the key was pressed. */
	bool released = false; /**< @brief True on the frame the key was released. */
	bool holding = false; /**< @brief True while the key is held beyond a short threshold. */
	float downTime = 0.0f; /**< @brief Time (in seconds) the key has been held down. */
};

/**
 * @brief Static input manager for handling keys, mouse, and scroll events.
 *
 * @details
 * Tracks the state of all registered keys and provides per-frame updates.
 * Supports querying @ref KeyInfo for any keycode, as well as registering
 * callbacks for mouse movement and scroll events.
 * 
 * Typical usage:
 * - Query key states with @ref GetKey().
 * - Register event callbacks with @ref RegisterMouseCallback() or @ref RegisterScrollCallback().
 * - Enable or disable mouse input with @ref TriggerMouse().
 */
class Input
{
	private:
		static bool mouseEnabled;
		static std::map<int, KeyInfo> keys;
		static std::vector<std::function<void (double, double)>> mouseCalls;
		static std::vector<std::function<void (double, double)>> scrollCalls;

		static double mx, my;

		static void AddKey(int keycode);

		static void UpdateKeys();

	public:
		/**
		 * @brief Gets the current state of a key.
		 * @param keycode Keycode to query.
		 * @return @ref KeyInfo struct describing key state (pressed, released, held, etc.).
		 */
		static KeyInfo GetKey(int keycode);

		static void Frame();

		static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void ScrollCallback(GLFWwindow* window, double deltaX, double deltaY);
		//static void ResizeCallback(GLFWwindow* window, int width, int height);

		/**
		 * @brief Registers a custom mouse movement callback.
		 * @param call Function to call on mouse movement (x, y).
		 */
		static void RegisterMouseCallback(std::function<void (double, double)> call);

		/**
		 * @brief Registers a custom scroll callback.
		 * @param call Function to call on scroll events (deltaX, deltaY).
		 */
		static void RegisterScrollCallback(std::function<void (double, double)> call);

		/**
		 * @brief Enables or disables mouse input capture.
		 * @param mode If -1, toggles state. Otherwise sets to enabled/disabled explicitly.
		 */
		static void TriggerMouse(int mode = -1);
};