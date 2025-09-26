#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/**
 * @file time.hpp
 * @brief Frame timing and time utilities.
 *
 * @details
 * Provides static time management for frame updates, including delta time,
 * second/tick intervals, and access to the current time. Intended for use
 * in rendering loops or simulations where consistent timing information is required.
 */

/**
 * @brief Static time manager for frame updates and timing intervals.
 *
 * @details
 * Tracks frame-to-frame delta time, identifies new "second" and "tick" intervals,
 * and provides access to the current system time.
 *
 * Typical usage:
 * - Use @ref deltaTime to get the duration of the last frame (in seconds).
 * - Use @ref newSecond or @ref newTick for periodic updates.
 * - Query @ref GetCurrentTime() to get the total elapsed time (in seconds).
 */
class Time
{
	private:
		static float deltaFrameTime;
		static float lastFrameTime;
		static float currentFrameTime;

		static float lastSecondInterval;
		static bool secondFrameInterval;
		static float lastTickInterval;
		static bool tickFrameInterval;

		static void SetFrameTimes();
		static void SetFrameIntervals();

	public:
		static const float& deltaTime; /**< @brief Delta time between frames (in seconds). */

		static const bool& newSecond; /**< @brief True if a new second started this frame. */
		static const bool& newTick; /**< @brief True if a new tick interval occurred this frame. */

		static void Frame();
		
		/**
		 * @brief Gets the current system time.
		 * @return Time in seconds since an implementation-defined epoch (e.g., app start).
		 */
		static double GetCurrentTime();
};