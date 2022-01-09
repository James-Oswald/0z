

#include<string>
#include<vulkan/vulkan.hpp>

class ShaderPipeline{
    private:
        const vk::Device& device;
        vk::ShaderModule createShader(const std::vector<char>* spvProgram);
    public:
        ShaderPipeline() = delete;
        ShaderPipeline(const vk::Device& device_, const std::vector<char>* vert, const std::vector<char>* tess, const std::vector<char>* geom, const std::vector<char>* frag);
        ~ShaderPipeline();
};