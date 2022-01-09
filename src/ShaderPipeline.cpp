
#include "ShaderPipeline.hpp"


vk::ShaderModule ShaderPipeline::createShader(const std::vector<char>* spvProgram){
    vk::ShaderModuleCreateInfo createInfo {};
    createInfo.codeSize = spvProgram->size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(spvProgram->data());
    return this->device.createShaderModule(createInfo);
}

ShaderPipeline::ShaderPipeline(
    const vk::Device& device_,
    const std::vector<char>* vert,
    const std::vector<char>* tess,
    const std::vector<char>* geom,
    const std::vector<char>* frag
):device(device_)
{
    this->createShader()
    vk::ShaderModule vert
}