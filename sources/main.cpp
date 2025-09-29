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
#include "object.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

Pass pass;
Pipeline pipeline;
Descriptor descriptor;

meshPNC32 quadMesh;
meshPNC32 cannonMesh;
meshPNC32 croissantMesh;

Image checkeredImageDiff;
Image checkeredImageNorm;
Image checkeredImageARM;

Image cannonImageDiff;
Image cannonImageNorm;
Image cannonImageARM;
//Image cannonImageRough;
//Image cannonImageMetal;
//Image cannonImageAO;

Image croissantImageDiff;
Image croissantImageNorm;
Image croissantImageARM;

Object checkeredFloor;
Object cannon;
Object croissant;

float angle = 0;

void Frame()
{
	if (Input::GetKey(GLFW_KEY_M).pressed)
	{
		Input::TriggerMouse();
	}

	Manager::GetCamera().UpdateView();

	angle += Time::deltaTime * 30.0;
	if (angle > 360) angle -= 360;

	checkeredFloor.GetData().viewPosition = Manager::GetCamera().GetPosition();
	checkeredFloor.GetData().view = Manager::GetCamera().GetView();
	checkeredFloor.GetData().projection = Manager::GetCamera().GetProjection();
	checkeredFloor.GetData().model = mat4::Identity();
	checkeredFloor.GetData().model.Scale(point3D(5, 5, 1));
	checkeredFloor.GetData().model.Rotate(-90, Axis::x);
	checkeredFloor.GetData().model.Translate(point3D(0.0, 0.0, 0.0));

	cannon.GetData().viewPosition = Manager::GetCamera().GetPosition();
	cannon.GetData().view = Manager::GetCamera().GetView();
	cannon.GetData().projection = Manager::GetCamera().GetProjection();
	cannon.GetData().model = mat4::Identity();
	cannon.GetData().model.Rotate(angle, Axis::y);
	cannon.GetData().model.Translate(point3D(0.0, 0.2, 0.0));

	croissant.GetData().viewPosition = Manager::GetCamera().GetPosition();
	croissant.GetData().view = Manager::GetCamera().GetView();
	croissant.GetData().projection = Manager::GetCamera().GetProjection();
	croissant.GetData().model = mat4::Identity();
	croissant.GetData().model.Rotate(angle + 45, Axis::y);
	croissant.GetData().model.Translate(point3D(2.0, 0.125, 0.0));
}

void Start()
{
	PassConfig passConfig = Pass::DefaultConfig(true);
	pass.Create(passConfig);

	PassInfo passInfo{};
	passInfo.pass = &pass;
	passInfo.useWindowExtent = true;
	
	Renderer::AddPass(passInfo);

	std::vector<DescriptorConfig> descriptorConfigs(4);
	descriptorConfigs[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorConfigs[0].stages = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorConfigs[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorConfigs[1].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorConfigs[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorConfigs[2].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorConfigs[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorConfigs[3].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	//descriptorConfigs[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//descriptorConfigs[4].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	//descriptorConfigs[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//descriptorConfigs[5].stages = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptor.Create(descriptorConfigs);

	quadMesh.Create(ShapeType::Quad);
	cannonMesh.Create(ModelLoader("cannon", ModelType::Gltf));
	croissantMesh.Create(ModelLoader("croissant", ModelType::Gltf));

	ImageConfig imageConfig = Image::DefaultConfig();
	ImageConfig imageNormalConfig = Image::DefaultNormalConfig();
	ImageConfig imageGreyscaleConfig = Image::DefaultGreyscaleConfig();

	checkeredImageDiff.Create(ImageLoader("checkered_diff", ImageType::Jpg), imageConfig);
	checkeredImageNorm.Create(ImageLoader("checkered_norm", ImageType::Jpg), imageNormalConfig);
	checkeredImageARM.Create(ImageLoader("checkered_arm", ImageType::Jpg), imageNormalConfig);

	cannonImageDiff.Create(ImageLoader("cannon_diff", ImageType::Jpg), imageConfig);
	cannonImageNorm.Create(ImageLoader("cannon_norm", ImageType::Jpg), imageNormalConfig);
	cannonImageARM.Create(ImageLoader("cannon_arm", ImageType::Jpg), imageNormalConfig);
	//cannonImageRough.Create(ImageLoader("cannon_rough", ImageType::Jpg), imageGreyscaleConfig);
	//cannonImageMetal.Create(ImageLoader("cannon_metal", ImageType::Jpg), imageGreyscaleConfig);
	//cannonImageAO.Create(ImageLoader("cannon_ao", ImageType::Jpg), imageGreyscaleConfig);

	croissantImageDiff.Create(ImageLoader("croissant_diff", ImageType::Jpg), imageConfig);
	croissantImageNorm.Create(ImageLoader("croissant_norm", ImageType::Jpg), imageNormalConfig);
	croissantImageARM.Create(ImageLoader("croissant_arm", ImageType::Jpg), imageNormalConfig);

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = cannonMesh.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	pipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	pipeline.Create(pipelineConfig);
	
	checkeredFloor.Create(quadMesh, checkeredImageDiff, pipeline, descriptor);
	cannon.Create(cannonMesh, cannonImageDiff, pipeline, descriptor);
	croissant.Create(croissantMesh, croissantImageDiff, pipeline, descriptor);

	descriptor.Update(checkeredFloor.GetSet(), 2, checkeredImageNorm);
	descriptor.Update(checkeredFloor.GetSet(), 3, checkeredImageARM);
	descriptor.Update(cannon.GetSet(), 2, cannonImageNorm);
	descriptor.Update(cannon.GetSet(), 3, cannonImageARM);
	//descriptor.Update(cannon.GetSet(), 3, cannonImageRough);
	//descriptor.Update(cannon.GetSet(), 4, cannonImageMetal);
	//descriptor.Update(cannon.GetSet(), 5, cannonImageAO);
	descriptor.Update(croissant.GetSet(), 2, croissantImageNorm);
	descriptor.Update(croissant.GetSet(), 3, croissantImageARM);

	Manager::GetCamera().Move(point3D(0, 1, -2));
}

void End()
{
	quadMesh.Destroy();
	cannonMesh.Destroy();
	croissantMesh.Destroy();
	
	checkeredImageDiff.Destroy();
	checkeredImageNorm.Destroy();
	checkeredImageARM.Destroy();

	cannonImageDiff.Destroy();
	cannonImageNorm.Destroy();
	cannonImageARM.Destroy();
	//cannonImageRough.Destroy();
	//cannonImageMetal.Destroy();
	//cannonImageAO.Destroy();

	croissantImageDiff.Destroy();
	croissantImageNorm.Destroy();
	croissantImageARM.Destroy();
	
	checkeredFloor.Destroy();
	cannon.Destroy();
	croissant.Destroy();

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