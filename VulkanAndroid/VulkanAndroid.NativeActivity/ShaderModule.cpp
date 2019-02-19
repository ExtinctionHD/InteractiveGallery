#include "ShaderModule.h"
#include "AssetManager.h"

ShaderModule::ShaderModule(Device *device, const std::string &path, VkShaderStageFlagBits stage)
    : device(device), stage(stage), data(nullptr), specializationInfo(nullptr)
{
	std::vector<uint8_t> code = AssetManager::getBytes(path);

	VkShaderModuleCreateInfo createInfo{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		code.size(),
		reinterpret_cast<uint32_t*>(code.data())
	};

    CALL_VK(vkCreateShaderModule(device->get(), &createInfo, nullptr, &module));
    LOGI("Shader module [%s] created.", path.c_str());
}

ShaderModule::ShaderModule(
	Device *device,
    const std::string &path,
    VkShaderStageFlagBits stage,
    std::vector<VkSpecializationMapEntry> entries,
    std::vector<const void*> data) : ShaderModule(device, path, stage)
{
    if (!entries.empty())
    {
        LOGA(entries.size() == data.size());

		const size_t size = entries.back().offset + entries.back().size;

		this->data = malloc(size);
		this->entries = entries;

		for (size_t i = 0; i < data.size(); i++)
		{
			memcpy(reinterpret_cast<uint8_t*>(this->data) + entries[i].offset, data[i], entries[i].size);
		}

		specializationInfo = new VkSpecializationInfo{
			uint32_t(entries.size()),
			this->entries.data(),
			size,
			this->data
		};    
    }
}

ShaderModule::~ShaderModule()
{
	delete specializationInfo;
    free(data);
	vkDestroyShaderModule(device->get(), module, nullptr);
}

VkShaderStageFlagBits ShaderModule::getStage() const
{
	return stage;
}

VkShaderModule ShaderModule::getModule() const
{
	return module;
}

VkSpecializationInfo* ShaderModule::getSpecializationInfo() const
{
	return specializationInfo;
}
