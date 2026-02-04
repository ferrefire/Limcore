#include "ui.hpp"

#include "manager.hpp"
#include "graphics.hpp"
#include "descriptor.hpp"
#include "renderer.hpp"

UI::UI()
{

}

UI::~UI()
{
	DestroyContext();
}

void UI::CreateContext(VkRenderPass renderPass, uint32_t subpass)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	io = &ImGui::GetIO();
	//io->ConfigFlags |= ImGuiConfigFlags_NoMouse;
	io->ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(Manager::GetWindow().GetData(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = Graphics::GetInstance();
	init_info.PhysicalDevice = Manager::GetDevice().GetPhysicalDevice();
	init_info.Device = Manager::GetDevice().GetLogicalDevice();
	init_info.QueueFamily = Manager::GetDevice().GetQueueIndex(QueueType::Graphics);
	init_info.Queue = Manager::GetDevice().GetQueue(init_info.QueueFamily);
	init_info.PipelineCache = nullptr;
	//init_info.DescriptorPool = Descriptor::GetPool();
	init_info.DescriptorPoolSize = Manager::GetSwapchain().GetFrameCount();
	//init_info.RenderPass = Renderer::GetPassInfo(0).pass->GetRenderpass();
	//init_info.Subpass = 1;
	init_info.RenderPass = renderPass;
	init_info.Subpass = subpass;
	init_info.MinImageCount = Manager::GetSwapchain().GetFrameCount();
	init_info.ImageCount = Manager::GetSwapchain().GetFrameCount();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;

	ImGui_ImplVulkan_Init(&init_info);

	Manager::RegisterFrameCall(Frame);
	Manager::RegisterEndCall(DestroyContext);
}

void UI::DestroyContext()
{
	if (!io) {return;}

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	io = nullptr;
}

ImGuiIO* UI::GetIO()
{
	return (io);
}

void UI::TriggerMouseInput(bool mode)
{
	if (!io) {return;}

	if (!mode) {io->ConfigFlags |= ImGuiConfigFlags_NoMouse;}
	else {io->ConfigFlags &= ~ImGuiConfigFlags_NoMouse;}
}

void UI::TriggerKeyboardInput(bool mode)
{
	if (!io) {return;}

	if (mode) {io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;}
	else {io->ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;}
}

void UI::Frame()
{
	if (!io) {return;}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	RenderFPS();

	ImGui::Begin("inspector");

	for (Menu& menu : menus)
	{
		if (ImGui::CollapsingHeader(menu.title.c_str())) {menu.RenderMenu();}
	}

	ImGui::End();

	ImGui::Render();
}

void UI::Render(VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
	if (!io) return;

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void UI::RenderFPS()
{
	if (!io) return;

	ImGui::Begin("statistics");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
	ImGui::End();
}

Menu& UI::NewMenu(std::string title)
{
	Menu menu;
	menu.title = title;

	menus.push_back(menu);

	return (menus[menus.size() - 1]);
}

void Menu::RenderMenu(int start, int end)
{
	static void(*nodeFunc)(void) = nullptr;

	if (start < 0) {start = 0;}
	if (end < 0 || end > components.size()) {end = components.size();}

	for (int i = start; i < end; i++)
	{
		if (components[i].first == ComponentType::FloatSlider) {floatSliders[components[i].second].Render();}
		else if (components[i].first == ComponentType::Button) {buttons[components[i].second].Render();}
		else if (components[i].first == ComponentType::Checkbox) {checkboxes[components[i].second].Render();}

		if (nodeFunc != nullptr && ImGui::IsItemEdited()) {nodeFunc();}

		if (components[i].first == ComponentType::Node)
		{
			int nodeStart = i + 1;
			int nodeEnd = FindNodeEnd(nodes[components[i].second].name, nodeStart);

			if (ImGui::TreeNode(nodes[components[i].second].name.c_str()))
			{
				nodeFunc = nodes[components[i].second].func;
				RenderMenu(nodeStart, nodeEnd);
				nodeFunc = nullptr;
				ImGui::TreePop();
			}

			i = nodeEnd;
		}
	}
}

void Menu::TriggerNode(std::string name, void(*func)(void))
{
	Node node;
	node.name = name;
	node.func = func;
	nodes.push_back(node);

	components.push_back({ComponentType::Node, nodes.size() - 1});
}

void Menu::AddSlider(std::string name, float &value, float min, float max)
{
	Slider<float> slider(name, value, min, max);

	floatSliders.push_back(slider);

	components.push_back({ComponentType::FloatSlider, floatSliders.size() - 1});
}

void Menu::AddButton(std::string name, void(*func)(void))
{
	Button button(name, func);

	buttons.push_back(button);

	components.push_back({ComponentType::Button, buttons.size() - 1});
}

void Menu::AddCheckbox(std::string name, bool &value)
{
	Checkbox checkbox(name, value);

	checkboxes.push_back(checkbox);

	components.push_back({ComponentType::Checkbox, checkboxes.size() - 1});
}

int Menu::FindNodeEnd(std::string name, int start)
{
	for (int i = start; i < components.size(); i++)
	{
		if (components[i].first == ComponentType::Node && nodes[components[i].second].name.compare(name) == 0) 
			{return (i);}
	}

	return (components.size());
}

ImGuiIO* UI::io = nullptr;
std::vector<Menu> UI::menus;
//int placeholder = 0;