//Put this first to utilize the precompiled vkfw.hpp header
#include<vkfw.hpp>

#define LOG
#define USE_VALIDATION_LAYERS

#ifdef USE_VALIDATION_LAYERS
#include<cstdlib>
#endif


#ifdef LOG
#include<iostream>
#endif

#include<set>
#include<cstdint>
#include<vector>
#include<string>
#include<optional>



/*QueueFamilyIndices::QueueFamilyIndices(std::optional<uint32_t> graphicsFamily_)
:graphicsFamily(graphicsFamily_)
{}*/

class Application{
    private:
        static void glfwErrorCallback(int, const char*);
        void initLibs();
        void selectPhysicalDevice();
#ifdef USE_VALIDATION_LAYERS        
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };
#endif
        const std::vector<const char*> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
    public:
        struct QueueFamilyInfo{
            float graphicsFamilyPriority = 1;
            float presentationFamilyPriority = 1;
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentationFamily; 
        };
        struct PhysicalDeviceInfo{ //Holds information about physical devices
            vk::PhysicalDevice physicalDevice;
            QueueFamilyInfo queueFamilyInfo;
            std::vector<vk::ExtensionProperties> extensionProps;
        };

        vkfw::Window window;
        vk::Instance instance;
        vk::ApplicationInfo appInfo;
        vk::InstanceCreateInfo createInfo;
        std::vector<PhysicalDeviceInfo> physicalDevices; //All Physical Devices
        PhysicalDeviceInfo physicalDeviceInfo;           //A ref to the selected physical device this app is running on.
        vk::Device logicalDevice;
        vk::Queue graphicsQueue;
        vk::SurfaceKHR surface;

        Application();
        ~Application();
        void mainLoop();
};

void Application::glfwErrorCallback(int errorCode, const char* error){
    throw std::runtime_error(std::string("GLFW encountered an error!\n ") + error + "\n");
}

void Application::initLibs(){
    //Validation Layer Setup
#ifdef USE_VALIDATION_LAYERS
    putenv("VK_LAYER_PATH=D:\\Software\\VulkanSDK\\Bin");
    putenv("VK_INSTANCE_LAYERS=\"VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation\"");
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
#ifdef LOG
    std::cout<<"Available Layers"<<"\n";
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

#ifdef LOG
    //Log available vulkan extensions
    std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();
    std::cout<<"Available Extensions"<<"\n";
    for(const auto& extension : extensions)
        std::cout<<"\t"<<extension.extensionName<<"\n";
#endif

    //Window Setup
    vkfw::setErrorCallback(&glfwErrorCallback);
    vkfw::init();
    vkfw::WindowHints hints {};
    hints.clientAPI = vkfw::ClientAPI::eNone;
    window = vkfw::createWindow(800, 600, "0z", hints);

    //Vulkan Setup
    appInfo = vk::ApplicationInfo();
    appInfo.pApplicationName = "Oz";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount;
    const char** glfwExtensions = vkfw::getRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
#ifdef USE_VALIDATION_LAYERS
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
    createInfo.enabledLayerCount = 0;
#endif
    vk::Result createInstanceResult = vk::createInstance(&createInfo, nullptr, &instance);
    if(createInstanceResult != vk::Result::eSuccess)
        throw std::runtime_error(std::string("VK Instance creation failed with code") + std::to_string((int)createInstanceResult));
    surface = vkfw::createWindowSurface(instance, window, nullptr);
    if(!(bool)surface)
        throw std::runtime_error("Failed to create surface");
}

void Application::selectPhysicalDevice(){
    //Physical Device Selection
    std::vector<vk::PhysicalDevice> allPhysicalDevices = instance.enumeratePhysicalDevices(); //All available graphics hardware
    if (allPhysicalDevices.size()==0) 
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
#ifdef LOG
    std::cout<<"Available Physical Graphics Hardware"<<"\n";
    for(const vk::PhysicalDevice& device : allPhysicalDevices){
        vk::PhysicalDeviceProperties props = device.getProperties();
        std::cout<<"\t"<<props.deviceName<<"\n";
        std::cout<<"\t\t"<<props.apiVersion<<"\n";
        std::cout<<"\t\t"<<props.driverVersion<<"\n";
        std::cout<<"\t\t"<<(int)props.deviceType<<"\n";
    }
#endif
    //Select GPUs based on desired features that are available
    for(const vk::PhysicalDevice& device : allPhysicalDevices){

        //We only allow discrte GPUs cause we're too cool for IG
        vk::PhysicalDeviceProperties deviceProps = device.getProperties();
        if(deviceProps.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
            continue;
        
        std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);
        if(!requiredExtensions.empty())
            continue;

        //We have to have graphics capabilities on our GPU and surface support
        std::vector<vk::QueueFamilyProperties> queueFamProps = device.getQueueFamilyProperties();
        QueueFamilyInfo queueFamInfo;
        for(int i = 0; i < queueFamProps.size(); i++){
            if(queueFamProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
                queueFamInfo.graphicsFamily = i;
            else if(device.getSurfaceSupportKHR(i, surface))
                queueFamInfo.presentationFamily = i;
        }
        if(!(queueFamInfo.graphicsFamily.has_value() && queueFamInfo.presentationFamily.has_value()))
            continue;

        PhysicalDeviceInfo physicalDeviceInfo;
        physicalDeviceInfo.physicalDevice = device;
        physicalDeviceInfo.extensionProps = availableExtensions;
        physicalDeviceInfo.queueFamilyInfo = queueFamInfo;
        physicalDevices.push_back(std::move(physicalDeviceInfo));
    }
    if(physicalDevices.size() == 0)
        throw std::runtime_error("Failed to find GPUs that meet criteria!");
    physicalDeviceInfo = physicalDevices[0];
}

Application::Application(){

    initLibs();
    selectPhysicalDevice();
    //Create a logical device
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = physicalDeviceInfo.queueFamilyInfo.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &physicalDeviceInfo.queueFamilyInfo.graphicsFamilyPriority;
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
#ifdef USE_VALIDATION_LAYERS //For backwards compatability
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#else
    deviceCreateInfo.enabledLayerCount = 0;
#endif
    logicalDevice = physicalDeviceInfo.physicalDevice.createDevice(deviceCreateInfo);
    graphicsQueue = logicalDevice.getQueue(physicalDeviceInfo.queueFamilyInfo.graphicsFamily.value(), 0);
}

Application::~Application(){
    logicalDevice.destroy();
    instance.destroySurfaceKHR(surface);
    instance.destroy();
    window.destroy();
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