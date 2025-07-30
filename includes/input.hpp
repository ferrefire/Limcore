#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <vector>
#include <functional>

struct KeyInfo
{
	bool down = false;
	bool pressed = false;
	bool released = false;
	bool holding = false;
	
	float downTime = 0.0f;
};

class Input
{
	private:
		static std::map<int, KeyInfo> keys;
		static std::vector<std::function<void (double, double)>> mouseCalls;
		static std::vector<std::function<void (double, double)>> scrollCalls;

		static double mx, my;

		static void AddKey(int keycode);

		static void UpdateKeys();

	public:
		static KeyInfo GetKey(int keycode);

		static void Frame();

		static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
		static void ScrollCallback(GLFWwindow* window, double deltaX, double deltaY);

		static void RegisterMouseCallback(std::function<void (double, double)> call);
		static void RegisterScrollCallback(std::function<void (double, double)> call);
};