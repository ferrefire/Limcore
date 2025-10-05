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
Pipeline realisticPipeline;
Descriptor descriptor;
Descriptor realisticDescriptor;

meshPNC32 quadMesh;
meshPNC32 cannonMesh;
meshPNC32 croissantMesh;
meshPNC32 lionMesh;

Image checkeredImageDiff;
Image checkeredImageNorm;
Image checkeredImageARM;

Image cannonImageDiff;
Image cannonImageNorm;
Image cannonImageARM;

Image croissantImageDiff;
Image croissantImageNorm;
Image croissantImageARM;

Image lionImageDiff;
Image lionImageNorm;
Image lionImageARM;

Image whiteImage;
Image blackImage;

Object checkeredFloor;
Object cannon;
Object croissant;
Object lion;

float angle = -45 + 180;

void Frame()
{
	if (Input::GetKey(GLFW_KEY_M).pressed)
	{
		Input::TriggerMouse();
	}

	Manager::GetCamera().UpdateView();

	angle += Time::deltaTime * 10.0;
	if (angle > 360) angle -= 360;

	Renderer::GetCurrentFrameData().viewPosition = Manager::GetCamera().GetPosition();
	Renderer::GetCurrentFrameData().view = Manager::GetCamera().GetView();
	Renderer::GetCurrentFrameData().projection = Manager::GetCamera().GetProjection();

	checkeredFloor.GetData().model = mat4::Identity();
	checkeredFloor.GetData().model.Scale(point3D(5, 5, 1));
	checkeredFloor.GetData().model.Rotate(-90, Axis::x);
	checkeredFloor.GetData().model.Translate(point3D(0.0, 0.0, 0.0));

	cannon.GetData().model = mat4::Identity();
	cannon.GetData().model.Rotate(angle, Axis::y);
	cannon.GetData().model.Translate(point3D(0.0, 0.2, 0.0));

	croissant.GetData().model = mat4::Identity();
	croissant.GetData().model.Rotate(angle + 45, Axis::y);
	croissant.GetData().model.Translate(point3D(2.0, 0.125, 0.0));

	lion.GetData().model = mat4::Identity();
	lion.GetData().model.Rotate(angle - 45, Axis::y);
	lion.GetData().model.Translate(point3D(-2.0, 0.5, 0.0));
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

	quadMesh.Create(ShapeType::Quad);
	cannonMesh.Create(ModelLoader("cannon", ModelType::Gltf));
	croissantMesh.Create(ModelLoader("croissant", ModelType::Gltf));
	lionMesh.Create(ModelLoader("lion_head", ModelType::Gltf));

	std::cout << "Mesh creation time: " << (Time::GetCurrentTime() - startTime) * 1000 << " ms." << std::endl << std::endl;

	ImageConfig imageConfig = Image::DefaultConfig();
	ImageConfig imageNormalConfig = Image::DefaultNormalConfig();
	ImageConfig imageGreyscaleConfig = Image::DefaultGreyscaleConfig();

	imageConfig.width = 1024;
	imageConfig.height = 1024;

	startTime = Time::GetCurrentTime();

	std::vector<ImageLoader*> loaders = ImageLoader::LoadImages({
		{"checkered_diff", ImageType::Jpg},
		{"checkered_norm", ImageType::Jpg},
		{"checkered_arm", ImageType::Jpg},
		{"cannon_diff", ImageType::Jpg},
		{"cannon_norm", ImageType::Jpg},
		{"cannon_arm", ImageType::Jpg},
		{"croissant_diff", ImageType::Jpg},
		{"croissant_norm", ImageType::Jpg},
		{"croissant_arm", ImageType::Jpg},
		{"lion_head_diff", ImageType::Jpg},
		{"lion_head_norm", ImageType::Jpg},
		{"lion_head_arm", ImageType::Jpg}
	});

	checkeredImageDiff.Create(*loaders[0], imageConfig);
	checkeredImageNorm.Create(*loaders[1], imageNormalConfig);
	checkeredImageARM.Create(*loaders[2], imageNormalConfig);
	cannonImageDiff.Create(*loaders[3], imageConfig);
	cannonImageNorm.Create(*loaders[4], imageNormalConfig);
	cannonImageARM.Create(*loaders[5], imageNormalConfig);
	croissantImageDiff.Create(*loaders[6], imageConfig);
	croissantImageNorm.Create(*loaders[7], imageNormalConfig);
	croissantImageARM.Create(*loaders[8], imageNormalConfig);
	lionImageDiff.Create(*loaders[9], imageConfig);
	lionImageNorm.Create(*loaders[10], imageNormalConfig);
	lionImageARM.Create(*loaders[11], imageNormalConfig);

	std::cout << "Total creation time: " << (Time::GetCurrentTime() - startTime) * 1000 << " ms." << std::endl;

	for (size_t i = 0; i < loaders.size(); i++)
	{
		delete (loaders[i]);
	}
	loaders.clear();

	ImageConfig placeholderImageConfig = Image::DefaultConfig();
	placeholderImageConfig.width = 1;
	placeholderImageConfig.height = 1;
	placeholderImageConfig.depth = 1;
	
	std::array<unsigned char, 4> pixels{255, 255, 255, 255};
	whiteImage.Create(placeholderImageConfig);
	whiteImage.Update(pixels.data(), pixels.size());

	PipelineConfig pipelineConfig = Pipeline::DefaultConfig();
	pipelineConfig.shader = "default";
	pipelineConfig.vertexInfo = cannonMesh.GetVertexInfo();
	pipelineConfig.renderpass = pass.GetRenderpass();
	//pipelineConfig.descriptorLayouts = { descriptor.GetLayout() };
	pipelineConfig.descriptorLayouts = Renderer::GetDescriptorLayouts();
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	pipelineConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	pipeline.Create(pipelineConfig);

	checkeredFloor.Create(quadMesh, pipeline);
	cannon.Create(cannonMesh, pipeline);
	croissant.Create(croissantMesh, pipeline);
	lion.Create(lionMesh, pipeline);

	Renderer::GetDescriptorSet(1).Update(checkeredFloor.GetMaterialSet(), 0, {&checkeredImageDiff, &checkeredImageNorm, &checkeredImageARM});
	Renderer::GetDescriptorSet(1).Update(cannon.GetMaterialSet(), 0, {&cannonImageDiff, &cannonImageNorm, &cannonImageARM});
	Renderer::GetDescriptorSet(1).Update(croissant.GetMaterialSet(), 0, {&croissantImageDiff, &croissantImageNorm, &croissantImageARM});
	Renderer::GetDescriptorSet(1).Update(lion.GetMaterialSet(), 0, {&lionImageDiff, &lionImageNorm, &lionImageARM});

	Manager::GetCamera().Move(point3D(0, 1, -2));
}

void End()
{
	quadMesh.Destroy();
	cannonMesh.Destroy();
	croissantMesh.Destroy();
	lionMesh.Destroy();
	
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
	//croissantImageRough.Destroy();
	//croissantImageAO.Destroy();

	lionImageDiff.Destroy();
	lionImageNorm.Destroy();
	lionImageARM.Destroy();

	whiteImage.Destroy();
	//blackImage.Destroy();
	
	checkeredFloor.Destroy();
	cannon.Destroy();
	croissant.Destroy();
	lion.Destroy();

	pass.Destroy();
	descriptor.Destroy();
	realisticDescriptor.Destroy();
	pipeline.Destroy();
	realisticPipeline.Destroy();
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