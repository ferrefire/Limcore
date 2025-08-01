#include "input.hpp"

#include "manager.hpp"
#include "time.hpp"

void Input::AddKey(int keycode)
{
	keys[keycode] = KeyInfo{};
}

KeyInfo Input::GetKey(int keycode)
{
	if (!keys.contains(keycode)) AddKey(keycode);

	return (keys[keycode]);
}

void Input::UpdateKeys()
{
	for (auto& key : keys)
	{
		bool down = key.second.down;

		key.second.down = glfwGetKey(Manager::GetWindow().GetData(), key.first) == GLFW_PRESS;
		key.second.pressed = (!down && key.second.down);
		key.second.released = (down && !key.second.down);

		if (key.second.pressed) key.second.downTime = 0.0f;
		if (key.second.down) key.second.downTime += Time::deltaTime;
		if (key.second.down && key.second.downTime > 0.25f) key.second.holding = true;
		if (key.second.holding && !key.second.down) key.second.holding = false;
	}
}

void Input::Frame()
{
	UpdateKeys();
}

void Input::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	double deltaX = mx - xpos, deltaY = ypos - my;
	
	for (std::function<void (double, double)> call : mouseCalls) { call(deltaX, deltaY); }

	mx = xpos;
	my = ypos;
}

void Input::ScrollCallback(GLFWwindow* window, double deltaX, double deltaY)
{
	for (std::function<void (double, double)> call : scrollCalls) { call(deltaX, deltaY); }
}

void Input::RegisterMouseCallback(std::function<void (double, double)> call)
{
	mouseCalls.push_back(call);
}

void Input::RegisterScrollCallback(std::function<void (double, double)> call)
{
	scrollCalls.push_back(call);
}

std::map<int, KeyInfo> Input::keys;
std::vector<std::function<void (double, double)>> Input::mouseCalls;
std::vector<std::function<void (double, double)>> Input::scrollCalls;

double Input::mx = 0, Input::my = 0;