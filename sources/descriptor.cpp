#include "descriptor.hpp"

#include "manager.hpp"
#include "printer.hpp"
#include "utilities.hpp"
#include "renderer.hpp"
#include "bitmask.hpp"

#include <stdexcept>

Descriptor::Descriptor()
{

}

Descriptor::~Descriptor()
{
	Destroy();
}

void Descriptor::Create(size_t layoutSet, const std::vector<DescriptorConfig>& descriptorConfig, Device* descriptorDevice)
{
	set = layoutSet;
	config = descriptorConfig;
	device = descriptorDevice;

	if (!device) device = &Manager::GetDevice();

	CreateLayout();
}

void Descriptor::CreateLayout()
{
	if (layout) throw (std::runtime_error("Descriptor layout already exists"));
	if (!device) throw (std::runtime_error("Descriptor has no device"));

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings(config.size());
	for (int i = 0; i < config.size(); i++)
	{
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = static_cast<VkDescriptorType>(config[i].type);
		layoutBindings[i].descriptorCount = config[i].count;
		layoutBindings[i].stageFlags = config[i].stages;

		//if (Bitmask::HasFlag(config[i].stages, VK_SHADER_STAGE_COMPUTE_BIT)) compute = true;
	}

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = CUI(config.size());
	createInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(device->GetLogicalDevice(), &createInfo, nullptr, &layout) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create descriptor layout"));
}

void Descriptor::AllocateSet(VkDescriptorSet& set)
{
	if (set) throw (std::runtime_error("Descriptor set is already allocated"));
	if (!layout) throw (std::runtime_error("Descriptor has no layout"));
	if (!pool) throw (std::runtime_error("Descriptor has no pool"));
	if (!device) throw (std::runtime_error("Descriptor has no device"));

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = pool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(device->GetLogicalDevice(), &allocateInfo, &set) != VK_SUCCESS)
		throw (std::runtime_error("Failed to allocate descriptor set"));
}

void Descriptor::Destroy()
{
	if (!device) return;

	sets.clear();

	if (layout)
	{
		vkDestroyDescriptorSetLayout(device->GetLogicalDevice(), layout, nullptr);
		layout = nullptr;
	}
}

const VkDescriptorSetLayout& Descriptor::GetLayout() const
{
	if (!layout) throw (std::runtime_error("Descriptor layout requested but does not exist"));

	return (layout);
}

const std::vector<DescriptorConfig>& Descriptor::GetConfig() const
{
	return (config);
}

size_t Descriptor::GetNewSet()
{
	//if (sets.size() >= 10) throw (std::runtime_error("Maximum amount of sets already allocated"));

	size_t setID = sets.size();
	sets.resize(setID + 1);

	AllocateSet(sets[setID]);

	return (setID);
}

size_t Descriptor::GetNewSetDynamic()
{
	size_t setID = GetNewSet();

	for (size_t i = 1; i < Renderer::GetFrameCount(); i++) { GetNewSet(); }

	return (setID);
}

void Descriptor::Bind(size_t setID, VkCommandBuffer commandBuffer, const Pipeline& pipeline, int offset)
{
	if (!commandBuffer) throw (std::runtime_error("Cannot bind descriptor because command buffer does not exist"));
	//if (!pipelineLayout) throw (std::runtime_error("Cannot bind descriptor because pipeline layout does not exist"));
	if (!sets[setID]) throw (std::runtime_error("Descriptor has no set"));

	uint32_t dynamicOffset = (offset >= 0 ? CUI(offset) : 0);

	vkCmdBindDescriptorSets(commandBuffer, (pipeline.GetConfig().type == PipelineType::Compute ? 
		VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS),
		pipeline.GetLayout(), set, 1, &sets[setID], (offset >= 0 ? 1 : 0), &dynamicOffset);
}

void Descriptor::BindDynamic(size_t baseSetID, VkCommandBuffer commandBuffer, const Pipeline& pipeline, int offset)
{
	Bind(baseSetID + Renderer::GetCurrentFrame(), commandBuffer, pipeline, offset);
}

void Descriptor::Update(size_t setID, uint32_t binding, VkDescriptorBufferInfo* bufferInfos, VkDescriptorImageInfo* imageInfos)
{
	if (!sets[setID]) throw (std::runtime_error("Descriptor has no set"));
	if (!device) throw (std::runtime_error("Descriptor has no device"));

	VkWriteDescriptorSet writeInfo{};
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.dstSet = sets[setID];
	writeInfo.dstBinding = binding;
	writeInfo.dstArrayElement = 0;
	writeInfo.descriptorType = static_cast<VkDescriptorType>(config[binding].type);
	writeInfo.descriptorCount = config[binding].count;
	writeInfo.pBufferInfo = bufferInfos;
	writeInfo.pImageInfo = imageInfos;

	vkUpdateDescriptorSets(device->GetLogicalDevice(), 1, &writeInfo, 0, nullptr);
}

void Descriptor::Update(size_t setID, uint32_t binding, const Buffer& buffer, size_t size)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer.GetBuffer();
	bufferInfo.range = size == 0 ? buffer.GetConfig().size : size;

	Update(setID, binding, {&bufferInfo}, {});
}

void Descriptor::UpdateDynamic(size_t baseSetID, uint32_t binding, const std::vector<Buffer*> buffers, size_t size)
{
	if (buffers.size() != Renderer::GetFrameCount()) throw (std::runtime_error("Cannot update dynamicly because buffer count does not match frame count"));

	for (size_t i = 0; i < buffers.size(); i++)
	{
		if (buffers[i] == nullptr) throw (std::runtime_error("Buffer cannot be null"));

		Update(baseSetID + i, binding, *buffers[i], size);
	}
}

void Descriptor::Update(size_t setID, uint32_t binding, const Image& image)
{
	VkDescriptorImageInfo imageInfo{};
	if (config[binding].type != DescriptorType::InputAttatchment) imageInfo.sampler = image.GetSampler();
	imageInfo.imageView = image.GetView();
	imageInfo.imageLayout = image.GetConfig().currentLayout;

	Update(setID, binding, {}, {&imageInfo});
}

void Descriptor::Update(size_t setID, uint32_t binding, const std::vector<Image*> images)
{
	std::vector<VkDescriptorImageInfo> imageInfos(images.size());

	for (size_t i = 0; i < images.size(); i++)
	{
		if (images[i] != nullptr)
		{
			imageInfos[i].sampler = images[i]->GetSampler();
			imageInfos[i].imageView = images[i]->GetView();
			imageInfos[i].imageLayout = images[i]->GetConfig().currentLayout;
		}
		else
		{
			imageInfos[i].sampler = VK_NULL_HANDLE;
			imageInfos[i].imageView = VK_NULL_HANDLE;
			//imageInfos[i].imageLayout = VK_NULL_HANDLE;
		}
	}

	Update(setID, binding, {}, imageInfos.data());
}

void Descriptor::CreatePools(Device* descriptorDevice)
{
	if (pool != nullptr) throw (std::runtime_error("Descriptor pool already exists"));
	if (!descriptorDevice) descriptorDevice = &Manager::GetDevice();
	if (!descriptorDevice) throw (std::runtime_error("Descriptor has no device"));

	VkDescriptorPoolSize poolSizes[] = 
	{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10},
	}; // Reduce these numbers!

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = CUI(std::size(poolSizes));
	createInfo.pPoolSizes = poolSizes;
	createInfo.maxSets = 50;
	//createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(descriptorDevice->GetLogicalDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create descriptor pool"));
}

void Descriptor::DestroyPools(Device* descriptorDevice)
{
	if (pool == nullptr) return;
	if (!descriptorDevice) descriptorDevice = &Manager::GetDevice();
	if (!descriptorDevice) return;

	vkDestroyDescriptorPool(descriptorDevice->GetLogicalDevice(), pool, nullptr);
	pool = nullptr;
}

VkDescriptorPool& Descriptor::GetPool()
{
	if (!pool) {throw (std::runtime_error("Pool requested but not yet created"));}

	return (pool);
}

std::ostream& operator<<(std::ostream& out, const DescriptorConfig& config)
{
	out << std::endl;
	out << ENUM_VAL(config.type) << std::endl;
	out << FLAG_VAL(config.stages, VkShaderStageFlagBits) << std::endl;
	out << VAR_VAL(config.count) << std::endl;

	return (out);
}

std::ostream& operator<<(std::ostream& out, const Descriptor& descriptor)
{
	for (const DescriptorConfig& config : descriptor.GetConfig())
	{
		out << config;
	}

	return (out);
}

VkDescriptorPool Descriptor::pool = nullptr;