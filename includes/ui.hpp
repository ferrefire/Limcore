#pragma once

#include "point.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <string>
#include <vector>

enum class ComponentType { Node = 0, FloatSlider = 1, Button = 2, Checkbox = 3, Dropdown = 4 };

#define COMPONENT_TEMPLATE template <typename T>

static int placeholder = 0;

struct Node
{
	std::string name = "node";
	void(*func)(void) = nullptr;
};

//struct Button
//{
//	std::string name = "button";
//	void(*func)(void) = nullptr;
//};

COMPONENT_TEMPLATE
class Component
{
	public:
		Component(std::string componentName, T& componentValue) : name(componentName), value(componentValue) {}
		~Component(){};

		std::string name = "component";
		T& value;

		virtual void Render() = 0;
};

COMPONENT_TEMPLATE
class Slider: public Component<T>
{
	public:
		Slider(std::string componentName, T& componentValue, T minValue, T maxValue) : Component<T>(componentName, componentValue)
		{
			min = minValue;
			max = maxValue;
		}
		~Slider(){};

		T min = 0;
		T max = 0;

		void Render() override
		{
			ImGui::PushItemWidth(250.0f);
			ImGui::SliderFloat(this->name.c_str(), &this->value, min, max);
			ImGui::PopItemWidth();
		}
};

class Button: public Component<int>
{
	public:
		Button(std::string componentName, void(*componentFunc)(void)) : Component<int>(componentName, placeholder)
		{
			func = componentFunc;
		}
		~Button(){};

		void(*func)(void) = nullptr;

		void Render() override
		{
			if (ImGui::Button(this->name.c_str())) {func();}
		}
};

class Checkbox: public Component<uint32_t>
{
	public:
		Checkbox(std::string componentName, uint32_t& componentValue) : Component<uint32_t>(componentName, componentValue)
		{
			
		}
		~Checkbox(){};

		void Render() override
		{
			ImGui::PushItemWidth(250.0f);
			ImGui::Checkbox(this->name.c_str(), reinterpret_cast<bool *>(&this->value));
			ImGui::PopItemWidth();
		}
};

class Dropdown: public Component<uint32_t>
{
	public:
		Dropdown(std::string componentName, uint32_t& componentValue, std::vector<std::string> componentOptions) : Component<uint32_t>(componentName, componentValue)
		{
			options = componentOptions;
		}
		~Dropdown(){};

		std::vector<std::string> options;

		void Render() override
		{
			std::vector<const char *> items(options.size());
			for (int i = 0; i < options.size(); i++) {items[i] = options[i].c_str();}

			ImGui::PushItemWidth(250.0f);
			ImGui::Combo(this->name.c_str(), reinterpret_cast<int *>(&this->value), items.data(), options.size());
			ImGui::PopItemWidth();
		}
};

class Menu
{
	private:
		std::vector<std::pair<ComponentType, int>> components;
		std::vector<Node> nodes;
		std::vector<Slider<float>> floatSliders;
		std::vector<Button> buttons;
		std::vector<Checkbox> checkboxes;
		std::vector<Dropdown> dropdowns;

	public:
		std::string title = "new menu";

		void RenderMenu(int start = -1, int end = -1);

		void TriggerNode(std::string name, void(*func)(void) = nullptr);
		void AddSlider(std::string name, float &value, float min, float max);
		void AddButton(std::string name, void(*func)(void));
		void AddCheckbox(std::string name, uint32_t &value);
		void AddDropdown(std::string name, uint32_t &value, std::vector<std::string> options);

		int FindNodeEnd(std::string name, int start);
};

class UI
{
	private:
		static ImGuiIO* io;
		static std::vector<Menu> menus;

		static void RenderFPS();

	public:
		UI();
		~UI();

		static void CreateContext(VkRenderPass renderPass, uint32_t subpass);
		static void DestroyContext();

		static ImGuiIO* GetIO();

		static void TriggerMouseInput(bool mode);
		static void TriggerKeyboardInput(bool mode);

		static void Frame();
		static void Render(VkCommandBuffer commandBuffer, uint32_t frameIndex);

		static Menu& NewMenu(std::string title);
};