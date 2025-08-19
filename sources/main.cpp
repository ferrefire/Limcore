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
//Mesh<Position | Normal, VK_INDEX_TYPE_UINT32> duck;
//Mesh<Position | Normal, VK_INDEX_TYPE_UINT32> croissant;
Pass pass;
Pipeline pipeline;
Descriptor descriptor;

UniformData hammerData;
Buffer hammerBuffer;
size_t hammerSet;

//UniformData duckData;
//Buffer duckBuffer;
//size_t duckSet;

//UniformData croissantData;
//Buffer croissantBuffer;
//size_t croissantSet;

float angle = 0;

void Start()
{
	Device* device = &Manager::GetDevice();

	//mesh.SetShape(Shape<Position | Normal, VK_INDEX_TYPE_UINT32>(ShapeType::Cube));
	Shape<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT16> shape;
	shape.Create(ModelLoader("rubber_duck_toy", ModelType::Gltf));
	//shape.Create(ShapeType::Cube);
	shape.Scalarize();
	hammer.SetShape(shape);
	hammer.Create(device);

	//shape.Create(ModelLoader("horse_statue", ModelType::Gltf));
	////shape.Create(ShapeType::Cube);
	//shape.Scalarize();
	//duck.SetShape(shape);
	//duck.Create(device);

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

	//duckData.model = mat4::Identity();
	//duckData.view = Manager::GetCamera().GetView();
	//duckData.projection = Manager::GetCamera().GetProjection();

	//croissantData.model = mat4::Identity();
	//croissantData.view = Manager::GetCamera().GetView();
	//croissantData.projection = Manager::GetCamera().GetProjection();

	BufferConfig bufferConfig{};
	bufferConfig.mapped = true;
	bufferConfig.size = sizeof(UniformData);

	hammerBuffer.Create(bufferConfig, device, &hammerData);
	//duckBuffer.Create(bufferConfig, device, &duckData);
	//croissantBuffer.Create(bufferConfig, device, &croissantData);

	std::vector<DescriptorConfig> descriptorConfigs(1);
	descriptorConfigs[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptor.Create(descriptorConfigs, device);

	hammerSet = descriptor.GetNewSet();
	//duckSet = descriptor.GetNewSet();
	//croissantSet = descriptor.GetNewSet();

	VkDescriptorBufferInfo hammerBufferInfo{};
	hammerBufferInfo.buffer = hammerBuffer.GetBuffer();
	hammerBufferInfo.range = sizeof(hammerData);
	descriptor.Update(hammerSet, 0, &hammerBufferInfo, nullptr);

	//VkDescriptorBufferInfo duckBufferInfo{};
	//duckBufferInfo.buffer = duckBuffer.GetBuffer();
	//duckBufferInfo.range = sizeof(duckData);
	//descriptor.Update(duckSet, 0, &duckBufferInfo, nullptr);

	//VkDescriptorBufferInfo croissantBufferInfo{};
	//croissantBufferInfo.buffer = croissantBuffer.GetBuffer();
	//croissantBufferInfo.range = sizeof(croissantData);
	//descriptor.Update(croissantSet, 0, &croissantBufferInfo, nullptr);

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = hammer.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	pipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	//pipelineConfig.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//pipelineConfig.rasterization.cullMode = VK_CULL_MODE_NONE;
	pipeline.Create(pipelineConfig, device);

	Renderer::AddPass(&pass);
}

void Frame(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	angle += Time::deltaTime * 60;

	Manager::GetCamera().UpdateView();

	mat4 rotation = mat4::Rotation(angle * 0.5, Axis::x);
	rotation.Rotate(angle * 2, Axis::y);
	rotation.Rotate(angle * 0.25, Axis::z);

	hammerData.view = Manager::GetCamera().GetView();
	hammerData.model = mat4::Identity();
	//hammerData.model *= rotation;
	hammerData.model.Rotate(angle + 45, Axis::y);
	hammerData.model.Translate(point3D(-2.0, 0, 2.0));
	hammerBuffer.Update(&hammerData, sizeof(UniformData));

	//duckData.view = Manager::GetCamera().GetView();
	//duckData.model = mat4::Identity();
	//duckData.model.Rotate(angle, Axis::y);
	//duckData.model.Translate(point3D(2.0, 0, 2.0));
	//duckBuffer.Update(&duckData, sizeof(UniformData));

	//croissantData.view = Manager::GetCamera().GetView();
	//croissantData.model = mat4::Identity();
	////croissantData.model.Rotate(angle, Axis::y);
	//croissantData.model.Rotate(angle + 135, Axis::y);
	//croissantData.model.Translate(point3D(0.0, 0, 2.0));
	//croissantBuffer.Update(&croissantData, sizeof(UniformData));

	pipeline.Bind(commandBuffer);

	descriptor.Bind(hammerSet, commandBuffer, pipeline.GetLayout());
	hammer.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(hammer.GetIndices().size()), 1, 0, 0, 0);

	//descriptor.Bind(duckSet, commandBuffer, pipeline.GetLayout());
	//duck.Bind(commandBuffer);
	//vkCmdDrawIndexed(commandBuffer, CUI(duck.GetIndices().size()), 1, 0, 0, 0);

	//descriptor.Bind(croissantSet, commandBuffer, pipeline.GetLayout());
	//croissant.Bind(commandBuffer);
	//vkCmdDrawIndexed(commandBuffer, CUI(croissant.GetIndices().size()), 1, 0, 0, 0);
}

void End()
{
	hammer.Destroy();
	//duck.Destroy();
	//croissant.Destroy();
	pass.Destroy();
	hammerBuffer.Destroy();
	//duckBuffer.Destroy();
	//croissantBuffer.Destroy();
	descriptor.Destroy();
	pipeline.Destroy();
}

int main(int argc, char** argv)
{
	//std::cout << sizeof(char) << std::endl;
	//std::cout << sizeof(uint8_t) << std::endl;

	//ImageLoader imageLoader("rubber_duck_toy_diff", ImageType::Jpg);

	//std::vector<int> vals = { 90, 40, 0, 5, 0, 0, 4, 0, 0, 0 };

	ImageLoader imageLoader("rubber_duck_toy_diff", ImageType::Jpg);
	//ImageLoader imageLoader("croissant_diff", ImageType::Jpg);

	exit(EXIT_SUCCESS);

	//uint8_t data[] = {1, 2, 3, 4, 5};
	//uint32_t data = 6;
	//ByteReader br((uint8_t*)&data, sizeof(data));
	//std::cout << (int)br.Read8() << std::endl;
	//std::cout << (int)br.Read8() << std::endl;
	//std::cout << (int)br.Read8() << std::endl;
	//std::cout << (int)br.Read8() << std::endl;
	//std::cout << (int)br.Read8() << std::endl;
	//std::cout << br.BytesLeft() << std::endl;

	//std::string str = "a[cd[aaaa]b]bb";
	//std::pair<size_t, size_t> range = Utilities::FindPair(str, 0, {'[', ']'});
	//std::cout << range.first << " " << range.second << std::endl;
	//exit(EXIT_SUCCESS);

	//std::string str = Utilities::FileToString(Utilities::GetPath() + "/resources/models/cannon.gltf");
	//std::pair<size_t, size_t> range = Utilities::FindPair(str, str.find("meshes"), {'[', ']'});
	//std::cout << range.first << " " << range.second << std::endl;
	//std::cout << str.substr(range.first + 1, range.second - range.first - 1) << std::endl;
	//exit(EXIT_SUCCESS);

	Manager::ParseArguments(argv, argc);
	Manager::Create();

	Manager::RegisterStartCall(Start);
	Renderer::RegisterCall(Frame);
	Manager::RegisterEndCall(End);

	Manager::Run();

	Manager::Destroy();

	exit(EXIT_SUCCESS);
}