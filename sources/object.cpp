#include "object.hpp"

#include "utilities.hpp"
#include "renderer.hpp"

Object::Object()
{

}

Object::~Object()
{
	Destroy();
}

void Object::Create(meshPNC32& objectMesh, Image& objectImage, Pipeline& objectPipeline, Descriptor& objectDescriptor)
{
	mesh = &objectMesh;
	image = &objectImage;
	pipeline = &objectPipeline;
	descriptor = &objectDescriptor;

	BufferConfig bufferConfig{};
	bufferConfig.mapped = true;
	bufferConfig.size = sizeof(UniformData);
	buffer.Create(bufferConfig, &data);

	set = descriptor->GetNewSet();

	descriptor->Update(set, 0, buffer);
	descriptor->Update(set, 1, *image);

	Renderer::RegisterCall(0, this, &Object::Render);
}

void Object::Destroy()
{
	buffer.Destroy();

	mesh = nullptr;
	image = nullptr;
	pipeline = nullptr;
	descriptor = nullptr;
}

UniformData& Object::GetData()
{
	return (data);
}

const size_t& Object::GetSet() const
{
	return (set);
}

void Object::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	if (!mesh || !image || !pipeline || !descriptor) throw (std::runtime_error("Object has invalid resources"));

	buffer.Update(&data, sizeof(UniformData));

	pipeline->Bind(commandBuffer);
	descriptor->Bind(set, commandBuffer, pipeline->GetLayout());
	mesh->Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(mesh->GetIndices().size()), 1, 0, 0, 0);
}