#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>

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

		static void AddKey(int keycode);

		static void UpdateKeys();

	public:
		static KeyInfo GetKey(int keycode);

		static void Frame();
};