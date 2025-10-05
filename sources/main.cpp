#include "manager.hpp"
#include "point.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "pass.hpp"
#include "pipeline.hpp"
#include "descriptor.hpp"
#include "buffer.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include "time.hpp"
#include "image.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

struct UniformData
{
	mat4 model;
	mat4 view;
	mat4 projection;
	point4D viewPosition;
};

UniformData data{};
Buffer buffer;

Pass pass;
Pipeline pipeline;
Descriptor descriptor;
size_t setID;

meshPNC32 cannonMesh;

Image cannonImageDiff;
Image cannonImageNorm;
Image cannonImageARM;

float angle = -45 + 180;

void Render(VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
	pipeline.Bind(commandBuffer);
	descriptor.Bind(setID, commandBuffer, pipeline.GetLayout());
	cannonMesh.Bind(commandBuffer);

	vkCmdDrawIndexed(commandBuffer, cannonMesh.GetIndices().size(), 1, 0, 0, 0);
}

void Frame()
{
	if (Input::GetKey(GLFW_KEY_M).pressed)
	{
		Input::TriggerMouse();
	}

	Manager::GetCamera().UpdateView();

	angle += Time::deltaTime * 10.0;
	if (angle > 360) angle -= 360;

	data.model = mat4::Identity();
	data.model.Rotate(angle, Axis::y);
	data.model.Translate(point3D(0.0, 0.2, 0.0));
	data.projection = Manager::GetCamera().GetProjection();
	data.view = Manager::GetCamera().GetView();
	data.viewPosition = Manager::GetCamera().GetPosition();

	buffer.Update(&data, sizeof(data));
}

void Start()
{
	PassConfig passConfig = Pass::DefaultConfig(true);
	pass.Create(passConfig);

	PassInfo passInfo{};
	passInfo.pass = &pass;
	passInfo.useWindowExtent = true;
	
	Renderer::AddPass(passInfo);

	double startTime = Time::GetCurrentTime();

	cannonMesh.Create(ModelLoader("cannon", ModelType::Gltf));
	
	//std::cout << "Mesh creation time: " << (Time::GetCurrentTime() - startTime) * 1000 << " ms." << std::endl << std::endl;

	ImageConfig imageConfig = Image::DefaultConfig();
	ImageConfig imageNormalConfig = Image::DefaultNormalConfig();
	ImageConfig imageGreyscaleConfig = Image::DefaultGreyscaleConfig();

	startTime = Time::GetCurrentTime();

	std::vector<ImageLoader*> loaders = ImageLoader::LoadImages({
		{"cannon_diff", ImageType::Jpg},
		{"cannon_norm", ImageType::Jpg},
		{"cannon_arm", ImageType::Jpg},
	});

	cannonImageDiff.Create(*loaders[0], imageConfig);
	cannonImageNorm.Create(*loaders[1], imageNormalConfig);
	cannonImageARM.Create(*loaders[2], imageNormalConfig);

	std::cout << "Total creation time: " << (Time::GetCurrentTime() - startTime) * 1000 << " ms." << std::endl;

	for (size_t i = 0; i < loaders.size(); i++)
	{
		delete (loaders[i]);
	}
	loaders.clear();

	BufferConfig bufferConfig{};
	bufferConfig.mapped = true;
	bufferConfig.size = sizeof(UniformData);
	buffer.Create(bufferConfig);

	std::vector<DescriptorConfig> descriptorConfigs(2);
	descriptorConfigs[0].type = DescriptorType::UniformBuffer;
	descriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorConfigs[1].type = DescriptorType::CombinedSampler;
	descriptorConfigs[1].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorConfigs[1].count = 3;
	descriptor.Create(0, descriptorConfigs);

	setID = descriptor.GetNewSet();
	descriptor.Update(setID, 0, buffer);
	descriptor.Update(setID, 1, {&cannonImageDiff, &cannonImageNorm, &cannonImageARM});

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = cannonMesh.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	pipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	pipeline.Create(pipelineConfig);

	Manager::GetCamera().Move(point3D(0, 1, -2));

	Renderer::RegisterCall(0, Render);
}

void End()
{
	cannonMesh.Destroy();

	buffer.Destroy();

	cannonImageDiff.Destroy();
	cannonImageNorm.Destroy();
	cannonImageARM.Destroy();

	pass.Destroy();
	descriptor.Destroy();
	pipeline.Destroy();
}

int main(int argc, char** argv)
{
	Manager::ParseArguments(argv, argc);
	Manager::Create();

	Manager::RegisterStartCall(Start);
	Manager::RegisterFrameCall(Frame);
	Manager::RegisterEndCall(End);

	Manager::Run();

	Manager::Destroy();

	exit(EXIT_SUCCESS);
}