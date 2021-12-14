
#define LOG
#define USEVALIDLAYERS

#ifdef USEVALIDLAYERS
#include<cstdlib>
#endif

#ifdef LOG
#include<iostream>
#endif

#include<cstdint>
#include<vector>
#include<string>
#include<optional>

#include<vkfw.hpp>
#include<vulkan/vulkan.hpp>

struct QueueFamilyIndices{
    //QueueFamilyIndices(std::optional<uint32_t>);
    float graphicsFamilyPriority;
    std::optional<uint32_t> graphicsFamily;
};

/*QueueFamilyIndices::QueueFamilyIndices(std::optional<uint32_t> graphicsFamily_)
:graphicsFamily(graphicsFamily_)
{}*/

class Application{
    public:
        vkfw::Window window;
        vk::Instance instance;
        vk::ApplicationInfo appInfo;
        vk::InstanceCreateInfo createInfo;
        std::vector<vk::PhysicalDevice> physicalDevices; //GPUs
        std::vector<QueueFamilyIndices> physicalDeviceQueueFamilies;
        std::vector<vk::Device> logicalDevices;

        Application();
        ~Application();
        void mainLoop();
};

Application::Application(){

    //Validation Layer Setup
#ifdef USEVALIDLAYERS
    putenv("VK_LAYER_PATH=D:\\Software\\VulkanSDK\\Bin");
    putenv("VK_INSTANCE_LAYERS=\"VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation\"");
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
#ifdef LOG
    std::cout<<"Available Validation Layers"<<"\n";
    for(const auto& layer : availableLayers)
        std::cout<<"\t"<< layer.layerName<<"\n";
#endif
    for(const char* layerName : validationLayers){
        bool layerFound = false;
        for(const auto& layerProperties : availableLayers)
            if (strcmp(layerName, layerProperties.layerName)==0){
                layerFound = true;
                break;
            }
        if (!layerFound)
            throw std::runtime_error(std::string("Debug Layer ")+layerName+" was requested but not found");
    }
#endif

    //Window Setup
    vkfw::init();
    vkfw::WindowHints hints;
    hints.clientAPI = vkfw::ClientAPI::eNone;
    window = vkfw::createWindow(800, 600, "0z", hints);

    //Vulkan Setup
    appInfo = vk::ApplicationInfo();
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount;
    const char** glfwExtensions = vkfw::getRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
#ifdef USEVALIDLAYERS
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif
    vk::Result createInstanceResult = vk::createInstance(&createInfo, nullptr, &instance);
    if(createInstanceResult != vk::Result::eSuccess)
        throw std::runtime_error(std::string("VK Instance creation failed with code") + std::to_string((int)createInstanceResult));
    
    //Extensioon selection?
    std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();
#ifdef LOG
    std::cout<<"Available Extensions"<<"\n";
    for(const auto& extension : extensions)
        std::cout<<"\t"<<extension.extensionName<<"\n";
#endif

    //Physical Device Selection
    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    if (devices.size()==0) 
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
#ifdef LOG
    std::cout<<"Available Physical Graphics Hardware"<<"\n";
    for(const vk::PhysicalDevice& device : devices){
        vk::PhysicalDeviceProperties props = device.getProperties();
        std::cout<<"\t"<<props.deviceName<<"\n";
        std::cout<<"\t\t"<<props.apiVersion<<"\n";
        std::cout<<"\t\t"<<props.driverVersion<<"\n";
        std::cout<<"\t\t"<<(int)props.deviceType<<"\n";
    }
#endif
    //Select GPUs based on desired features that are available
    for(const vk::PhysicalDevice& device : devices){
        vk::PhysicalDeviceProperties deviceProps = device.getProperties();
        if(deviceProps.deviceType == vk::PhysicalDeviceType::eDiscreteGpu){ //We only allow discrte GPUs cause we're dumb
            std::vector<vk::QueueFamilyProperties> queueFamProps = device.getQueueFamilyProperties();
            QueueFamilyIndices indices {};
            for(int i = 0; i < queueFamProps.size(); i++)
                if(queueFamProps[i].queueFlags & vk::QueueFlagBits::eGraphics){
                    indices.graphicsFamily = i;
                    break;
                }
            if(indices.graphicsFamily.has_value()){  //We have to have graphics capabilities on our GPU
                physicalDevices.push_back(device);
                indices.graphicsFamilyPriority = 1.0f;
                physicalDeviceQueueFamilies.push_back(indices);
            }
        }
    }
    if(physicalDevices.size() == 0)
        throw std::runtime_error("Failed to find GPUs that meet criteria!");
    for(int i = 0; i < physicalDevices.size(); i++){
        const vk::PhysicalDevice& device = physicalDevices[i];
        const QueueFamilyIndices& indices = physicalDeviceQueueFamilies[i];
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &indices.graphicsFamilyPriority;
        vk::PhysicalDeviceFeatures deviceFeatures;
        vk::DeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = 0;
#ifdef USEVALIDLAYERS //For backwards compatability
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
        deviceCreateInfo.enabledLayerCount = 0;
#endif
        logicalDevices.push_back(device.createDevice(deviceCreateInfo));
    }
}

Application::~Application(){
    instance.destroy();
    window.destroy();
    for(vk::Device dev : logicalDevices)
        dev.destroy();
    vkfw::terminate();
}

void Application::mainLoop(){
    while(!window.shouldClose()){
        vkfw::pollEvents();
    }
}

int main(){
    Application a;
    a.mainLoop();
    return 0;
}