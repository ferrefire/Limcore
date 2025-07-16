#include "time.hpp"

void Time::Frame()
{
	SetFrameTimes();
	SetFrameIntervals();
}

void Time::SetFrameTimes()
{
	currentFrameTime = glfwGetTime();
	deltaFrameTime = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;
}

void Time::SetFrameIntervals()
{
	if (secondFrameInterval) secondFrameInterval = false;
	if (currentFrameTime - lastSecondInterval > 1.0)
	{
		lastSecondInterval = currentFrameTime;
		secondFrameInterval = true;
	}

	if (tickFrameInterval) tickFrameInterval = false;
	if (currentFrameTime - lastTickInterval > 0.1)
	{
		lastTickInterval = currentFrameTime;
		tickFrameInterval = true;
	}
}

float Time::deltaFrameTime = 0.0f;
float Time::lastFrameTime = 0.0f;
float Time::currentFrameTime = 0.0f;

float Time::lastSecondInterval = 0.0f;
bool Time::secondFrameInterval = 0.0f;
float Time::lastTickInterval = 0.0f;
bool Time::tickFrameInterval = 0.0f;

const float& Time::deltaTime = Time::deltaFrameTime;

const bool& Time::newSecond = Time::secondFrameInterval;
const bool& Time::newTick = Time::tickFrameInterval;