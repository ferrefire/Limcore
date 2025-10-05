#include "descriptor.hpp"

#include "manager.hpp"
#include "printer.hpp"
#include "utilities.hpp"

#include <stdexcept>

Descriptor::Descriptor()
{

}

Descriptor::~Descriptor()
{
	Destroy();
}

void Descriptor::Create(const std::vector<DescriptorConfig>& descriptorConfig, Device* descriptorDevice)
{
	config = descriptorConfig;
	device = descriptorDevice;

	if (!device) device = &Manager::GetDevice();

	CreateLayout();
	CreatePool();
}

void Descriptor::CreateLayout()
{
	if (layout) throw (std::runtime_error("Descriptor layout already exists"));
	if (!device) throw (std::runtime_error("Descriptor has no device"));

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings(config.size());
	for (int i = 0; i < config.size(); i++)
	{
		layoutBindings[i].binding = i;
		layoutBindings[i].descriptorType = config[i].type;
		layoutBindings[i].descriptorCount = config[i].count;
		layoutBindings[i].stageFlags = config[i].stages;
	}

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = CUI(config.size());
	createInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(device->GetLogicalDevice(), &createInfo, nullptr, &layout) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create descriptor layout"));
}

void Descriptor::CreatePool()
{
	if (pool) throw (std::runtime_error("Descriptor pool already exists"));
	if (!device) throw (std::runtime_error("Descriptor has no device"));

	std::vector<VkDescriptorPoolSize> poolSizes(config.size());
	for (int i = 0; i < config.size(); i++)
	{
		poolSizes[i].type = config[i].type;
		poolSizes[i].descriptorCount = config[i].count * 10;
	}

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = CUI(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();
	createInfo.maxSets = 10; //Make value dynamic

	if (vkCreateDescriptorPool(device->GetLogicalDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw (std::runtime_error("Failed to create descriptor pool"));
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

	if (pool)
	{
		vkDestroyDescriptorPool(device->GetLogicalDevice(), pool, nullptr);
		pool = nullptr;

		sets.clear();
	}

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
	if (sets.size() >= 10) throw (std::runtime_error("Maximum amount of sets already allocated"));

	size_t setID = sets.size();
	sets.resize(setID + 1);

	AllocateSet(sets[setID]);

	return (setID);
}

void Descriptor::Bind(size_t setIndex, size_t setID, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int offset)
{
	if (!commandBuffer) throw (std::runtime_error("Cannot bind descriptor because command buffer does not exist"));
	if (!pipelineLayout) throw (std::runtime_error("Cannot bind descriptor because pipeline layout does not exist"));
	if (!sets[setID]) throw (std::runtime_error("Descriptor has no set"));

	uint32_t dynamicOffset = (offset >= 0 ? CUI(offset) : 0);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, setIndex, 1, &sets[setID], (offset >= 0 ? 1 : 0), &dynamicOffset);
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
	writeInfo.descriptorType = config[binding].type;
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

void Descriptor::Update(size_t setID, uint32_t binding, const Image& image)
{
	VkDescriptorImageInfo imageInfo{};
	imageInfo.sampler = image.GetSampler();
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