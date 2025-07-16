#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
		static const float& deltaTime;

		static const bool& newSecond;
		static const bool& newTick;

		static void Frame();
};