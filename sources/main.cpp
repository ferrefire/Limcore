#include "manager.hpp"
#include "point.hpp"
#include "matrix.hpp"
#include "utilities.hpp"
#include "mesh.hpp"
#include "pass.hpp"
#include "pipeline.hpp"
#include "shape.hpp"
#include "device.hpp"
#include "renderer.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "time.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "structures.hpp"
#include "object.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

struct UniformData2
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

meshPNC32 cubeMesh;
meshPNC32 cannonMesh;
meshPNC32 hammerMesh;
meshPNC16 duckMesh;
meshPNC16 croissantMesh;
meshPC16 quadMesh;

Pass pass;

Pipeline pipeline;
Pipeline quadPipeline;
Descriptor descriptor;

UniformData2 quadData;
Buffer quadBuffer;
size_t quadSet;

UniformData2 duckData;
Buffer duckBuffer;
size_t duckSet;

UniformData2 croissantData;
Buffer croissantBuffer;
size_t croissantSet;

Image hammerImage;
Image duckImage;
Image croissantImage;
Image cannonImage;

Object cube;
Object cannon;
Object hammer;

float angle = 0;

void Frame()
{
	if (Input::GetKey(GLFW_KEY_M).pressed)
	{
		Input::TriggerMouse();
	}

	angle += Time::deltaTime * 60;

	Manager::GetCamera().UpdateView();

	mat4 rotation = mat4::Rotation(angle * 0.5, Axis::x);
	rotation.Rotate(angle * 2, Axis::y);
	rotation.Rotate(angle * 0.25, Axis::z);

	hammer.GetData().view = Manager::GetCamera().GetView();
	hammer.GetData().projection = Manager::GetCamera().GetProjection();
	hammer.GetData().model = mat4::Identity();
	hammer.GetData().model.Rotate(angle + 45, Axis::y);
	hammer.GetData().model.Translate(point3D(-2.0, 0, 2.0));

	quadData.view = Manager::GetCamera().GetView();
	quadData.projection = Manager::GetCamera().GetProjection();
	quadData.model = mat4::Identity();
	quadData.model.Scale(point3D(3, 3, 1));
	quadData.model.Translate(point3D(0.0, 0, -2.0));
	quadBuffer.Update(&quadData, sizeof(UniformData2));

	duckData.view = Manager::GetCamera().GetView();
	duckData.projection = Manager::GetCamera().GetProjection();
	duckData.model = mat4::Identity();
	duckData.model.Rotate(angle, Axis::y);
	duckData.model.Translate(point3D(2.0, 0, 2.0));
	duckBuffer.Update(&duckData, sizeof(UniformData2));

	croissantData.view = Manager::GetCamera().GetView();
	croissantData.projection = Manager::GetCamera().GetProjection();
	croissantData.model = mat4::Identity();
	croissantData.model.Rotate(angle + 135, Axis::y);
	croissantData.model.Translate(point3D(0.0, 0, 2.0));
	croissantBuffer.Update(&croissantData, sizeof(UniformData2));

	cube.GetData().view = Manager::GetCamera().GetView();
	cube.GetData().projection = Manager::GetCamera().GetProjection();
	cube.GetData().model = mat4::Identity();
	cube.GetData().model.Rotate(angle, Axis::y);
	cube.GetData().model.Translate(point3D(0.0, 0, 4.0));

	cannon.GetData().view = Manager::GetCamera().GetView();
	cannon.GetData().projection = Manager::GetCamera().GetProjection();
	cannon.GetData().model = mat4::Identity();
	cannon.GetData().model.Rotate(angle + 90, Axis::y);
	cannon.GetData().model.Translate(point3D(2.0, 0, 4.0));
}

void Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	pipeline.Bind(commandBuffer);

	descriptor.Bind(duckSet, commandBuffer, pipeline.GetLayout());
	duckMesh.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(duckMesh.GetIndices().size()), 1, 0, 0, 0);

	descriptor.Bind(croissantSet, commandBuffer, pipeline.GetLayout());
	croissantMesh.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(croissantMesh.GetIndices().size()), 1, 0, 0, 0);

	quadPipeline.Bind(commandBuffer);
	descriptor.Bind(quadSet, commandBuffer, quadPipeline.GetLayout());
	quadMesh.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(quadMesh.GetIndices().size()), 1, 0, 0, 0);
}

void Start()
{
	//hammerMesh.Create(ModelLoader("wooden_hammer", ModelType::Gltf));
	duckMesh.Create(ModelLoader("rubber_duck_toy", ModelType::Gltf));
	croissantMesh.Create(ModelLoader("croissant", ModelType::Gltf));
	quadMesh.Create(ShapeType::Quad);

	PassConfig passConfig = Pass::DefaultConfig(true);
	pass.Create(passConfig);

	BufferConfig bufferConfig{};
	bufferConfig.mapped = true;
	bufferConfig.size = sizeof(UniformData2);

	//hammerBuffer.Create(bufferConfig, &hammerData);
	duckBuffer.Create(bufferConfig, &duckData);
	croissantBuffer.Create(bufferConfig, &croissantData);
	quadBuffer.Create(bufferConfig, &quadData);

	ImageConfig imageConfig{};
	imageConfig.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageConfig.viewConfig = Image::DefaultViewConfig();

	hammerImage.Create(ImageLoader("wooden_hammer_diff", ImageType::Jpg), imageConfig);
	duckImage.Create(ImageLoader("rubber_duck_toy_diff", ImageType::Jpg), imageConfig);
	croissantImage.Create(ImageLoader("croissant_diff", ImageType::Jpg), imageConfig);
	cannonImage.Create(ImageLoader("cannon_diff", ImageType::Jpg), imageConfig);

	std::vector<DescriptorConfig> descriptorConfigs(2);
	descriptorConfigs[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorConfigs[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorConfigs[1].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptor.Create(descriptorConfigs);

	//hammerSet = descriptor.GetNewSet();
	duckSet = descriptor.GetNewSet();
	croissantSet = descriptor.GetNewSet();
	quadSet = descriptor.GetNewSet();

	//descriptor.Update(hammerSet, 0, hammerBuffer);
	descriptor.Update(duckSet, 0, duckBuffer);
	descriptor.Update(croissantSet, 0, croissantBuffer);
	descriptor.Update(quadSet, 0, quadBuffer);

	//descriptor.Update(hammerSet, 1, hammerImage);
	descriptor.Update(duckSet, 1, duckImage);
	descriptor.Update(croissantSet, 1, croissantImage);
	descriptor.Update(quadSet, 1, croissantImage);

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = duckMesh.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	pipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	pipeline.Create(pipelineConfig);

	PipelineConfig quadPipelineConfig = Pipeline::DefaultConfig();
	quadPipelineConfig.shader = "textureQuad";
	quadPipelineConfig.vertexInfo = quadMesh.GetVertexInfo();
	quadPipelineConfig.renderpass = pass.GetRenderpass();
	quadPipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	quadPipelineConfig.rasterization.cullMode = VK_CULL_MODE_NONE;
	quadPipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	quadPipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	quadPipeline.Create(quadPipelineConfig);

	PassInfo passInfo{};
	passInfo.pass = &pass;
	passInfo.useWindowExtent = true;

	Renderer::AddPass(passInfo);
	Renderer::RegisterCall(0, Render);

	cubeMesh.Create(ShapeType::Cube);
	cube.Create(cubeMesh, croissantImage, pipeline, descriptor);

	cannonMesh.Create(ModelLoader("cannon", ModelType::Gltf));
	cannon.Create(cannonMesh, cannonImage, pipeline, descriptor);

	hammerMesh.Create(ModelLoader("wooden_hammer", ModelType::Gltf));
	hammer.Create(hammerMesh, hammerImage, pipeline, descriptor);
}

void End()
{
	hammerMesh.Destroy();
	quadMesh.Destroy();
	duckMesh.Destroy();
	croissantMesh.Destroy();
	pass.Destroy();
	//hammerBuffer.Destroy();
	quadBuffer.Destroy();
	duckBuffer.Destroy();
	croissantBuffer.Destroy();
	hammerImage.Destroy();
	duckImage.Destroy();
	croissantImage.Destroy();
	cannonImage.Destroy();
	cubeMesh.Destroy();
	cube.Destroy();
	cannonMesh.Destroy();
	cannon.Destroy();
	hammer.Destroy();
	descriptor.Destroy();
	pipeline.Destroy();
	quadPipeline.Destroy();
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