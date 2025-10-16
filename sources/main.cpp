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
#include "utilities.hpp"
#include "command.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

struct UniformData
{
	mat4 view;
	mat4 projection;
	point4D viewPosition;
	point4D lightDirection;
};

UniformData data{};
std::vector<mat4> models(3);
std::vector<Buffer> frameBuffers;
std::vector<Buffer> objectBuffers;

Pass pass;
Pipeline pipeline;
Descriptor frameDescriptor;
Descriptor materialDescriptor;
Descriptor objectDescriptor;

Pipeline computePipeline;
Descriptor computeDescriptor;
Image computeImage;

meshPNC32 cannonMesh;
meshPNC32 quadMesh;
meshPNC32 lionMesh;

Image cannonImageDiff;
Image cannonImageNorm;
Image cannonImageARM;

Image woodImageDiff;
Image woodImageNorm;
Image woodImageARM;

Image lionImageDiff;
Image lionImageNorm;
Image lionImageARM;

float angle = -45 + 180;

void Render(VkCommandBuffer commandBuffer, uint32_t frameIndex)
{
	frameDescriptor.BindDynamic(0, commandBuffer, pipeline.GetLayout());
	pipeline.Bind(commandBuffer);

	materialDescriptor.Bind(0, commandBuffer, pipeline.GetLayout());
	objectDescriptor.BindDynamic(0, commandBuffer, pipeline.GetLayout(), 0);
	cannonMesh.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, cannonMesh.GetIndices().size(), 1, 0, 0, 0);

	materialDescriptor.Bind(1, commandBuffer, pipeline.GetLayout());
	objectDescriptor.BindDynamic(0, commandBuffer, pipeline.GetLayout(), 1 * sizeof(mat4));
	quadMesh.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, quadMesh.GetIndices().size(), 1, 0, 0, 0);

	materialDescriptor.Bind(2, commandBuffer, pipeline.GetLayout());
	objectDescriptor.BindDynamic(0, commandBuffer, pipeline.GetLayout(), 2 * sizeof(mat4));
	lionMesh.Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, lionMesh.GetIndices().size(), 1, 0, 0, 0);
}

void Frame()
{
	if (Input::GetKey(GLFW_KEY_M).pressed)
	{
		Input::TriggerMouse();
	}

	Manager::GetCamera().UpdateView();

	data.view = Manager::GetCamera().GetView();
	data.viewPosition = Manager::GetCamera().GetPosition();

	frameBuffers[Renderer::GetCurrentFrame()].Update(&data, sizeof(data));

	angle += Time::deltaTime * 10.0;
	if (angle > 360) angle -= 360;

	models[0] = mat4::Identity();
	models[0].Rotate(angle, Axis::y);
	models[0].Translate(point3D(0.0, 0.1875, 0.0));

	models[1] = mat4::Identity();
	models[1].Rotate(-90, Axis::x);

	models[2] = mat4::Identity();
	models[2].Rotate(angle + 45, Axis::y);
	models[2].Translate(point3D(2.0, 0.5, 0.0));

	objectBuffers[Renderer::GetCurrentFrame()].Update(models.data(), sizeof(mat4) * models.size());

	if (Input::GetKey(GLFW_KEY_P).pressed)
	{
		Command computeCommand;
		CommandConfig commandConfig{};
		commandConfig.queueIndex = Manager::GetDevice().GetQueueIndex(QueueType::Graphics);
		computeCommand.Create(commandConfig);
		computeCommand.Begin();

		computeDescriptor.Bind(0, computeCommand.GetBuffer(), computePipeline.GetLayout());
		computePipeline.Bind(computeCommand.GetBuffer());
		vkCmdDispatch(computeCommand.GetBuffer(), 1024 / 8, 1024 / 8, 1);

		computeCommand.End();
		computeCommand.Submit();
		computeCommand.Destroy();

		std::cout << "Compute shader executed." << std::endl;
	}
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
	lionMesh.Create(ModelLoader("lion_head", ModelType::Gltf));

	shapePNC32 quadShape(ShapeType::Quad);
	quadShape.Scale(point3D(5, 5, 1), true);
	quadMesh.Create(quadShape);
	
	//std::cout << "Mesh creation time: " << (Time::GetCurrentTime() - startTime) * 1000 << " ms." << std::endl << std::endl;

	ImageConfig imageConfig = Image::DefaultConfig();
	imageConfig.createMipmaps = true;
	imageConfig.samplerConfig.anisotropyEnabled = VK_TRUE;
	imageConfig.samplerConfig.maxAnisotropy = 8;
	ImageConfig imageNormalConfig = Image::DefaultNormalConfig();
	imageNormalConfig.createMipmaps = true;
	imageNormalConfig.samplerConfig.anisotropyEnabled = VK_TRUE;
	imageNormalConfig.samplerConfig.maxAnisotropy = 8;
	ImageConfig imageGreyscaleConfig = Image::DefaultGreyscaleConfig();

	startTime = Time::GetCurrentTime();

	std::vector<ImageLoader*> loaders = ImageLoader::LoadImages({
		{"cannon_diff", ImageType::Jpg},
		{"cannon_norm", ImageType::Jpg},
		{"cannon_arm", ImageType::Jpg},
		{"wood_diff", ImageType::Jpg},
		{"wood_norm", ImageType::Jpg},
		{"wood_arm", ImageType::Jpg},
		{"lion_head_diff", ImageType::Jpg},
		{"lion_head_norm", ImageType::Jpg},
		{"lion_head_arm", ImageType::Jpg},
	});

	cannonImageDiff.Create(*loaders[0], imageConfig);
	cannonImageNorm.Create(*loaders[1], imageNormalConfig);
	cannonImageARM.Create(*loaders[2], imageNormalConfig);
	woodImageDiff.Create(*loaders[3], imageConfig);
	woodImageNorm.Create(*loaders[4], imageNormalConfig);
	woodImageARM.Create(*loaders[5], imageNormalConfig);
	lionImageDiff.Create(*loaders[6], imageConfig);
	lionImageNorm.Create(*loaders[7], imageNormalConfig);
	lionImageARM.Create(*loaders[8], imageNormalConfig);

	std::cout << "Total creation time: " << (Time::GetCurrentTime() - startTime) * 1000 << " ms." << std::endl;

	for (size_t i = 0; i < loaders.size(); i++)
	{
		delete (loaders[i]);
	}
	loaders.clear();

	ImageConfig imageStorageConfig = Image::DefaultStorageConfig();
	imageStorageConfig.width = 1024;
	imageStorageConfig.height = 1024;
	computeImage.Create(imageStorageConfig);

	data.projection = Manager::GetCamera().GetProjection();
	data.lightDirection = point4D(point3D(0.2, 0.5, -0.4).Unitized());

	BufferConfig frameBufferConfig{};
	frameBufferConfig.mapped = true;
	frameBufferConfig.size = sizeof(UniformData);
	frameBuffers.resize(Renderer::GetFrameCount());
	for (Buffer& buffer : frameBuffers) { buffer.Create(frameBufferConfig); }

	BufferConfig objectBufferConfig{};
	objectBufferConfig.mapped = true;
	objectBufferConfig.size = sizeof(mat4) * models.size();
	objectBuffers.resize(Renderer::GetFrameCount());
	for (Buffer& buffer : objectBuffers) { buffer.Create(objectBufferConfig); }

	std::vector<DescriptorConfig> frameDescriptorConfigs(1);
	frameDescriptorConfigs[0].type = DescriptorType::UniformBuffer;
	frameDescriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	frameDescriptor.Create(0, frameDescriptorConfigs);

	std::vector<DescriptorConfig> materialDescriptorConfigs(1);
	materialDescriptorConfigs[0].type = DescriptorType::CombinedSampler;
	materialDescriptorConfigs[0].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialDescriptorConfigs[0].count = 3;
	materialDescriptor.Create(1, materialDescriptorConfigs);

	std::vector<DescriptorConfig> objectDescriptorConfigs(1);
	objectDescriptorConfigs[0].type = DescriptorType::DynamicUniformBuffer;
	objectDescriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT;
	objectDescriptor.Create(2, objectDescriptorConfigs);

	frameDescriptor.GetNewSetDynamic();
	frameDescriptor.UpdateDynamic(0, 0, Utilities::Pointerize(frameBuffers));

	materialDescriptor.GetNewSet();
	materialDescriptor.Update(0, 0, {&cannonImageDiff, &cannonImageNorm, &cannonImageARM});
	materialDescriptor.GetNewSet();
	materialDescriptor.Update(1, 0, {&woodImageDiff, &woodImageNorm, &woodImageARM});
	materialDescriptor.GetNewSet();
	materialDescriptor.Update(2, 0, {&lionImageDiff, &lionImageNorm, &lionImageARM});

	objectDescriptor.GetNewSetDynamic();
	objectDescriptor.UpdateDynamic(0, 0, Utilities::Pointerize(objectBuffers), sizeof(mat4));

	std::vector<DescriptorConfig> computeDescriptorConfigs(1);
	computeDescriptorConfigs[0].type = DescriptorType::StorageImage;
	computeDescriptorConfigs[0].stages = VK_SHADER_STAGE_COMPUTE_BIT;
	computeDescriptor.Create(0, computeDescriptorConfigs);

	computeDescriptor.GetNewSet();
	computeDescriptor.Update(0, 0, computeImage);

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = cannonMesh.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	pipelineConfig.descriptorLayouts = { frameDescriptor.GetLayout(), materialDescriptor.GetLayout(), objectDescriptor.GetLayout() };
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	pipeline.Create(pipelineConfig);

	PipelineConfig computePipelineConfig{};
	computePipelineConfig.shader = "heightmap";
	computePipelineConfig.type = PipelineType::Compute;
	computePipelineConfig.descriptorLayouts = { computeDescriptor.GetLayout() };
	computePipeline.Create(computePipelineConfig);

	Manager::GetCamera().Move(point3D(0, 1, -2));

	Renderer::RegisterCall(0, Render);
}

void End()
{
	cannonMesh.Destroy();
	quadMesh.Destroy();
	lionMesh.Destroy();

	for (Buffer& buffer : frameBuffers) { buffer.Destroy(); }
	frameBuffers.clear();

	for (Buffer& buffer : objectBuffers) { buffer.Destroy(); }
	objectBuffers.clear();

	cannonImageDiff.Destroy();
	cannonImageNorm.Destroy();
	cannonImageARM.Destroy();
	woodImageDiff.Destroy();
	woodImageNorm.Destroy();
	woodImageARM.Destroy();
	lionImageDiff.Destroy();
	lionImageNorm.Destroy();
	lionImageARM.Destroy();
	computeImage.Destroy();

	pass.Destroy();
	frameDescriptor.Destroy();
	materialDescriptor.Destroy();
	objectDescriptor.Destroy();
	computeDescriptor.Destroy();
	pipeline.Destroy();
	computePipeline.Destroy();
}

int main(int argc, char** argv)
{
	Manager::ParseArguments(argv, argc);
	Manager::GetConfig().deviceConfig.anisotropic = true;
	//Manager::GetConfig().framesInFlight = 3;
	Manager::Create();

	Manager::RegisterStartCall(Start);
	Manager::RegisterFrameCall(Frame);
	Manager::RegisterEndCall(End);

	Manager::Run();

	Manager::Destroy();

	exit(EXIT_SUCCESS);
}