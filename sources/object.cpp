#include "object.hpp"

#include "utilities.hpp"

Object::Object()
{

}

Object::~Object()
{
	Destroy();
}

void Object::Create(meshPNC32& objectMesh, Pipeline& objectPipeline)
{
	mesh = &objectMesh;
	pipeline = &objectPipeline;

	//BufferConfig bufferConfig{};
	//bufferConfig.mapped = true;
	//bufferConfig.size = sizeof(UniformData);
	//buffer.Create(bufferConfig, &data);

	//set = descriptor->GetNewSet();
	set = Renderer::RegisterObject();
	materialSet = Renderer::GetDescriptorSet(1).GetNewSet();

	//descriptor->Update(set, 0, buffer);
	//descriptor->Update(set, 1, *image);

	Renderer::RegisterCall(0, this, &Object::Render);
}

void Object::Destroy()
{
	//buffer.Destroy();

	mesh = nullptr;
	pipeline = nullptr;
}

UniformObjectData& Object::GetData()
{
	return (Renderer::GetCurrentObjectData(set));
}

const size_t& Object::GetSet() const
{
	return (set);
}

const size_t& Object::GetMaterialSet() const
{
	return (materialSet);
}

void Object::Render(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{
	if (!mesh || !pipeline) throw (std::runtime_error("Object has invalid resources"));

	//buffer.Update(&data, sizeof(UniformData));

	pipeline->Bind(commandBuffer);
	//descriptor->Bind(set, commandBuffer, pipeline->GetLayout());

	Renderer::BindMaterialData(materialSet);
	Renderer::BindObjectData(set);

	mesh->Bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, CUI(mesh->GetIndices().size()), 1, 0, 0, 0);
}