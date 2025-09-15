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
Mesh<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT32> hammer;
Mesh<Position | Coordinate, VK_INDEX_TYPE_UINT16> quad;
//Mesh<Position | Normal, VK_INDEX_TYPE_UINT32> duck;
//Mesh<Position | Normal, VK_INDEX_TYPE_UINT32> croissant;
Pass pass;

Pipeline pipeline;
Descriptor descriptor;

Pipeline quadPipeline;

UniformData hammerData;
Buffer hammerBuffer;
size_t hammerSet;

UniformData quadData;
Buffer quadBuffer;
size_t quadSet;

//UniformData duckData;
//Buffer duckBuffer;
//size_t duckSet;

//UniformData croissantData;
//Buffer croissantBuffer;
//size_t croissantSet;

Image image;

ImageLoader imageLoader("cannon_diff", ImageType::Jpg);
bool loaded = false;
bool updating = false;

float angle = 0;

void Start()
{
	Device* device = &Manager::GetDevice();

	//mesh.SetShape(Shape<Position | Normal, VK_INDEX_TYPE_UINT32>(ShapeType::Cube));
	Shape<Position | Normal | Coordinate, VK_INDEX_TYPE_UINT32> shape;
	shape.Create(ModelLoader("cannon", ModelType::Gltf));
	//shape.Create(ShapeType::Cube);
	shape.Scalarize();
	hammer.SetShape(shape);
	hammer.Create(device);

	Shape<Position | Coordinate, VK_INDEX_TYPE_UINT16> quadShape;
	quadShape.Create(ShapeType::Quad);
	quadShape.Scalarize();
	quad.SetShape(quadShape);
	quad.Create(device);

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

	quadData.model = mat4::Identity();
	quadData.view = Manager::GetCamera().GetView();
	quadData.projection = Manager::GetCamera().GetProjection();

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
	quadBuffer.Create(bufferConfig, device, &quadData);

	ImageConfig imageConfig{};
	imageConfig.targetLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageConfig.width = 1024;
	imageConfig.height = 1024;
	imageConfig.viewConfig = Image::DefaultViewConfig();
	image.Create(imageConfig, device);

	std::array<unsigned char, (1024 * 1024) * 4> pixels{};
	for (size_t i = 0; i < (1024 * 1024) * 4; i++)
	{ 
		pixels[i] = (i < 2097152 ? 255 : 0);
		pixels[++i] = 0;
		pixels[++i] = (i >= 2097152 ? 255 : 0);
		pixels[++i] = 255;
	}
	image.Update(&pixels[0], (1024 * 1024) * 4, {1024, 1024, 1});

	//std::array<unsigned char, (256 * 256) * 4> square{};
	//for (size_t i = 0; i < (256 * 256) * 4; i++)
	//{ 
	//	square[i] = 0;
	//	square[++i] = 255;
	//	square[++i] = 0;
	//	square[++i] = 255;
	//}
	//image.Update(&square[0], (256 * 256) * 4, {256, 256, 1}, {384, 384, 0});

	std::vector<DescriptorConfig> descriptorConfigs(2);
	descriptorConfigs[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorConfigs[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorConfigs[1].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptor.Create(descriptorConfigs, device);

	hammerSet = descriptor.GetNewSet();
	//duckSet = descriptor.GetNewSet();
	//croissantSet = descriptor.GetNewSet();
	quadSet = descriptor.GetNewSet();

	VkDescriptorBufferInfo hammerBufferInfo{};
	hammerBufferInfo.buffer = hammerBuffer.GetBuffer();
	hammerBufferInfo.range = sizeof(hammerData);
	descriptor.Update(hammerSet, 0, &hammerBufferInfo, nullptr);

	VkDescriptorBufferInfo quadBufferInfo{};
	quadBufferInfo.buffer = quadBuffer.GetBuffer();
	quadBufferInfo.range = sizeof(quadData);
	descriptor.Update(quadSet, 0, &quadBufferInfo, nullptr);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = image.GetSampler();
	imageInfo.imageView = image.GetView();
	imageInfo.imageLayout = image.GetConfig().currentLayout;
	descriptor.Update(hammerSet, 1, nullptr, &imageInfo);
	descriptor.Update(quadSet, 1, nullptr, &imageInfo);

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

	PipelineConfig quadPipelineConfig = Pipeline::DefaultConfig();
	quadPipelineConfig.shader = "textureQuad";
	quadPipelineConfig.vertexInfo = quad.GetVertexInfo();
	quadPipelineConfig.renderpass = pass.GetRenderpass();
	quadPipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	quadPipelineConfig.rasterization.cullMode = VK_CULL_MODE_NONE;
	quadPipeline.Create(quadPipelineConfig, device);

	Renderer::AddPass(&pass);
}

void Frame(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	static size_t xp = 0;
	static size_t yp = 0;

	/*if (yp < 1024)
	{
		std::array<unsigned char, 256 * 4> pixels{};

		for (size_t i = 0; i < 256 * 4; i++)
		{
			pixels[i] = 0;
			pixels[++i] = 255;
			pixels[++i] = 0;
			pixels[++i] = 255;
		}

		image.Update(&pixels[0], 256 * 4, {256, 1, 1}, { xp, yp, 0 });
		xp += 256;
		if (xp >= 1024)
		{
			xp = 0;
			yp += 1;
		}
	}*/

	if (Input::GetKey(GLFW_KEY_M).pressed)
	{
		Input::TriggerMouse();
	}

	if (Input::GetKey(GLFW_KEY_L).pressed)
	{		
		if (!loaded)
		{
			loaded = true;
			imageLoader.LoadEntropyData();
		}
		else if (!updating)
		{
			updating = true;
			std::vector<unsigned char> pixels{};
			imageLoader.LoadPixels(pixels);
			image.Update(&pixels[0], pixels.size(), {1024, 1024, 1});
		}
	}
	/*else if (loaded && !updating && Input::GetKey(GLFW_KEY_P).pressed)
	{
		updating = true;
	}
	else if (updating && yp < 64)
	{
		for (size_t i = 0; i < 16; i++)
		{
			if (yp < 64)
			{
				std::array<unsigned char, (16 * 16) * 4> pixels{};
				imageLoader.LoadBlock(pixels, yp * 64 + xp);
				image.Update(&pixels[0], (16 * 16) * 4, {16, 16, 1}, {xp * 16, yp * 16, 0});

				xp += 1;
				if (xp >= 64)
				{
					xp = 0;
					yp += 1;
				}
			}
		}

		yp = 64;
		std::vector<unsigned char> pixels{};
		imageLoader.LoadPixels(pixels);
		image.Update(&pixels[0], pixels.size(), {1024, 1024, 1});
	}*/

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

	quadData.view = Manager::GetCamera().GetView();
	quadData.model = mat4::Identity();
	//quadData.model.Rotate(angle + 45, Axis::y);
	quadData.model.Translate(point3D(0.0, 0, 2.0));
	quadBuffer.Update(&quadData, sizeof(UniformData));

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

	quadPipeline.Bind(commandBuffer);
	descriptor.Bind(quadSet, commandBuffer, quadPipeline.GetLayout());
	quad.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(quad.GetIndices().size()), 1, 0, 0, 0);

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
	quad.Destroy();
	//duck.Destroy();
	//croissant.Destroy();
	pass.Destroy();
	hammerBuffer.Destroy();
	quadBuffer.Destroy();
	//duckBuffer.Destroy();
	//croissantBuffer.Destroy();
	image.Destroy();
	descriptor.Destroy();
	pipeline.Destroy();
	quadPipeline.Destroy();
}

int main(int argc, char** argv)
{
	//std::cout << sizeof(char) << std::endl;
	//std::cout << sizeof(uint8_t) << std::endl;

	//ImageLoader imageLoader("rubber_duck_toy_diff", ImageType::Jpg);

	//std::vector<int> vals = { 90, 40, 0, 5, 0, 0, 4, 0, 0, 0 };

	//std::cout << std::stoi("010", nullptr, 2) << std::endl;
	//exit(EXIT_SUCCESS);

	/*std::array<int, 64> test{};

	test[0] = 1;
	bool check = true;

	for (size_t i = 1; i < 64; i++)
	{
		if (check)
		{
			check = false;
			for (size_t j = 0; j < 16; j++)
			{
				test[i] = 2;
				if (j + 1 < 16) i++;
			}
			continue;
		}

		test[i] = i + 1;
	}

	for (size_t i = 0; i < 64; i++)
	{
		std::cout << test[i] << ", ";
		if ((i + 1) % 8 == 0) std::cout << std::endl;
	}

	exit(EXIT_SUCCESS);*/

	//ImageLoader imageLoader("rubber_duck_toy_diff", ImageType::Jpg);
	////ImageLoader imageLoader("croissant_diff", ImageType::Jpg);
	//std::cout << imageLoader.GetInfo() << std::endl;
	//imageLoader.LoadEntropyData();
	//exit(EXIT_SUCCESS);

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

	//std::cout << imageLoader.GetInfo() << std::endl;

	Manager::ParseArguments(argv, argc);
	Manager::Create();

	Manager::RegisterStartCall(Start);
	Renderer::RegisterCall(Frame);
	Manager::RegisterEndCall(End);

	Manager::Run();

	Manager::Destroy();

	exit(EXIT_SUCCESS);
}