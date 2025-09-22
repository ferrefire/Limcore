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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

struct UniformData
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

//Mesh<Position | Normal, VK_INDEX_TYPE_UINT16> mesh;
Mesh<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> hammer;
Mesh<Position | Coordinate, VK_INDEX_TYPE_UINT16> quad;
Mesh<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> duck;
Mesh<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> croissant;
Pass pass;

Pipeline pipeline;
Pipeline quadPipeline;
Descriptor descriptor;

UniformData hammerData;
Buffer hammerBuffer;
size_t hammerSet;

UniformData quadData;
Buffer quadBuffer;
size_t quadSet;

UniformData duckData;
Buffer duckBuffer;
size_t duckSet;

UniformData croissantData;
Buffer croissantBuffer;
size_t croissantSet;

Image hammerImage;
Image duckImage;
Image croissantImage;

float angle = 0;

void Frame(VkCommandBuffer commandBuffer, uint32_t currentFrame)
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

	hammerData.view = Manager::GetCamera().GetView();
	hammerData.projection = Manager::GetCamera().GetProjection();
	hammerData.model = mat4::Identity();
	//hammerData.model *= rotation;
	hammerData.model.Rotate(angle + 45, Axis::y);
	hammerData.model.Translate(point3D(-2.0, 0, 2.0));
	hammerBuffer.Update(&hammerData, sizeof(UniformData));

	quadData.view = Manager::GetCamera().GetView();
	quadData.projection = Manager::GetCamera().GetProjection();
	quadData.model = mat4::Identity();
	//quadData.model.Rotate(angle + 45, Axis::y);
	quadData.model.Translate(point3D(0.0, 0, -2.0));
	quadBuffer.Update(&quadData, sizeof(UniformData));

	duckData.view = Manager::GetCamera().GetView();
	duckData.projection = Manager::GetCamera().GetProjection();
	duckData.model = mat4::Identity();
	duckData.model.Rotate(angle, Axis::y);
	duckData.model.Translate(point3D(2.0, 0, 2.0));
	duckBuffer.Update(&duckData, sizeof(UniformData));

	croissantData.view = Manager::GetCamera().GetView();
	croissantData.projection = Manager::GetCamera().GetProjection();
	croissantData.model = mat4::Identity();
	//croissantData.model.Rotate(angle, Axis::y);
	croissantData.model.Rotate(angle + 135, Axis::y);
	croissantData.model.Translate(point3D(0.0, 0, 2.0));
	croissantBuffer.Update(&croissantData, sizeof(UniformData));

	pipeline.Bind(commandBuffer);
	descriptor.Bind(hammerSet, commandBuffer, pipeline.GetLayout());
	hammer.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(hammer.GetIndices().size()), 1, 0, 0, 0);

	descriptor.Bind(duckSet, commandBuffer, pipeline.GetLayout());
	duck.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(duck.GetIndices().size()), 1, 0, 0, 0);

	descriptor.Bind(croissantSet, commandBuffer, pipeline.GetLayout());
	croissant.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(croissant.GetIndices().size()), 1, 0, 0, 0);

	quadPipeline.Bind(commandBuffer);
	descriptor.Bind(quadSet, commandBuffer, quadPipeline.GetLayout());
	quad.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(quad.GetIndices().size()), 1, 0, 0, 0);
}

void Start()
{
	Device* device = &Manager::GetDevice();

	//mesh.SetShape(Shape<Position | Normal, VK_INDEX_TYPE_UINT32>(ShapeType::Cube));
	Shape<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> shape;
	shape.Create(ModelLoader("wooden_hammer", ModelType::Gltf));
	//shape.Create(ShapeType::Cube);
	shape.Scalarize();
	hammer.SetShape(shape);
	hammer.Create(device);

	Shape<Position | Coordinate, VK_INDEX_TYPE_UINT16> quadShape;
	quadShape.Create(ShapeType::Quad);
	quadShape.Scalarize();
	quad.SetShape(quadShape);
	quad.Create(device);

	shape.Create(ModelLoader("rubber_duck_toy", ModelType::Gltf));
	//shape.Create(ShapeType::Cube);
	shape.Scalarize();
	duck.SetShape(shape);
	duck.Create(device);

	shape.Create(ModelLoader("croissant", ModelType::Gltf));
	shape.Scalarize();
	croissant.SetShape(shape);
	croissant.Create(device);

	//Shape<Position | Normal, VK_INDEX_TYPE_UINT32> shape32;
	//shape32.Create(ModelLoader("camera", ModelType::Gltf));
	//shape32.Scalarize();
	//croissant.SetShape(shape32);
	//croissant.Create(device);

	PassConfig passConfig = Pass::DefaultConfig(true);
	pass.Create(passConfig, device);

	hammerData.model = mat4::Identity();
	hammerData.view = Manager::GetCamera().GetView();
	hammerData.projection = Manager::GetCamera().GetProjection();

	quadData.model = mat4::Identity();
	quadData.view = Manager::GetCamera().GetView();
	quadData.projection = Manager::GetCamera().GetProjection();

	duckData.model = mat4::Identity();
	duckData.view = Manager::GetCamera().GetView();
	duckData.projection = Manager::GetCamera().GetProjection();

	croissantData.model = mat4::Identity();
	croissantData.view = Manager::GetCamera().GetView();
	croissantData.projection = Manager::GetCamera().GetProjection();

	BufferConfig bufferConfig{};
	bufferConfig.mapped = true;
	bufferConfig.size = sizeof(UniformData);

	hammerBuffer.Create(bufferConfig, device, &hammerData);
	duckBuffer.Create(bufferConfig, device, &duckData);
	croissantBuffer.Create(bufferConfig, device, &croissantData);
	quadBuffer.Create(bufferConfig, device, &quadData);

	ImageConfig imageConfig{};
	imageConfig.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageConfig.viewConfig = Image::DefaultViewConfig();

	hammerImage.Create("wooden_hammer_diff", imageConfig, device);
	duckImage.Create("rubber_duck_toy_diff", imageConfig, device);
	croissantImage.Create("croissant_diff", imageConfig, device);

	std::vector<DescriptorConfig> descriptorConfigs(2);
	descriptorConfigs[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorConfigs[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorConfigs[1].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptor.Create(descriptorConfigs, device);

	hammerSet = descriptor.GetNewSet();
	duckSet = descriptor.GetNewSet();
	croissantSet = descriptor.GetNewSet();
	quadSet = descriptor.GetNewSet();

	VkDescriptorBufferInfo hammerBufferInfo{};
	hammerBufferInfo.buffer = hammerBuffer.GetBuffer();
	hammerBufferInfo.range = sizeof(hammerData);
	descriptor.Update(hammerSet, 0, &hammerBufferInfo, nullptr);

	VkDescriptorBufferInfo duckBufferInfo{};
	duckBufferInfo.buffer = duckBuffer.GetBuffer();
	duckBufferInfo.range = sizeof(duckData);
	descriptor.Update(duckSet, 0, &duckBufferInfo, nullptr);

	VkDescriptorBufferInfo croissantBufferInfo{};
	croissantBufferInfo.buffer = croissantBuffer.GetBuffer();
	croissantBufferInfo.range = sizeof(croissantData);
	descriptor.Update(croissantSet, 0, &croissantBufferInfo, nullptr);

	VkDescriptorBufferInfo quadBufferInfo{};
	quadBufferInfo.buffer = quadBuffer.GetBuffer();
	quadBufferInfo.range = sizeof(quadData);
	descriptor.Update(quadSet, 0, &quadBufferInfo, nullptr);

	VkDescriptorImageInfo hammerImageInfo{};
	hammerImageInfo.sampler = hammerImage.GetSampler();
	hammerImageInfo.imageView = hammerImage.GetView();
	hammerImageInfo.imageLayout = hammerImage.GetConfig().currentLayout;
	descriptor.Update(hammerSet, 1, nullptr, &hammerImageInfo);

	VkDescriptorImageInfo duckImageInfo{};
	duckImageInfo.sampler = duckImage.GetSampler();
	duckImageInfo.imageView = duckImage.GetView();
	duckImageInfo.imageLayout = duckImage.GetConfig().currentLayout;
	descriptor.Update(duckSet, 1, nullptr, &duckImageInfo);
	descriptor.Update(quadSet, 1, nullptr, &duckImageInfo);

	VkDescriptorImageInfo croissantImageInfo{};
	croissantImageInfo.sampler = croissantImage.GetSampler();
	croissantImageInfo.imageView = croissantImage.GetView();
	croissantImageInfo.imageLayout = croissantImage.GetConfig().currentLayout;
	descriptor.Update(croissantSet, 1, nullptr, &croissantImageInfo);

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = hammer.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	pipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	pipeline.Create(pipelineConfig, device);

	PipelineConfig quadPipelineConfig = Pipeline::DefaultConfig();
	quadPipelineConfig.shader = "textureQuad";
	quadPipelineConfig.vertexInfo = quad.GetVertexInfo();
	quadPipelineConfig.renderpass = pass.GetRenderpass();
	quadPipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	quadPipelineConfig.rasterization.cullMode = VK_CULL_MODE_NONE;
	quadPipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	quadPipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	quadPipeline.Create(quadPipelineConfig, device);

	PassInfo passInfo{};
	passInfo.pass = &pass;
	passInfo.useWindowExtent = true;

	Renderer::AddPass(passInfo);
	Renderer::RegisterCall(0, Frame);
}

void End()
{
	hammer.Destroy();
	quad.Destroy();
	duck.Destroy();
	croissant.Destroy();
	pass.Destroy();
	hammerBuffer.Destroy();
	quadBuffer.Destroy();
	duckBuffer.Destroy();
	croissantBuffer.Destroy();
	hammerImage.Destroy();
	duckImage.Destroy();
	croissantImage.Destroy();
	descriptor.Destroy();
	pipeline.Destroy();
	quadPipeline.Destroy();
}

int main(int argc, char** argv)
{
	Manager::ParseArguments(argv, argc);
	Manager::Create();

	Manager::RegisterStartCall(Start);
	Manager::RegisterEndCall(End);
	Manager::RegisterResizeCall([&]() { pass.Recreate(); });

	Manager::Run();

	Manager::Destroy();

	exit(EXIT_SUCCESS);
}