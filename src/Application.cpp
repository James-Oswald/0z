//Put this first to utilize the precompiled vkfw.hpp.gch header
#include<master.hpp> //precomiled 
#include<vkfw.hpp>
#include<boost/property_tree/ptree.hpp> //For the json config file
#include<boost/property_tree/json_parser.hpp>
#include<boost/dll/runtime_symbol_info.hpp>
#include<boost/filesystem.hpp>

#include<cstdlib>
#include<set>
#include<cstdint>
#include<vector>
#include<string>
#include<optional>

#include"Application.hpp"
#include"Utils.hpp"

VkBool32 Application::vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
    Application* self = (Application*)pUserData;
    Logger::Level level;
    switch(messageSeverity){
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            level = Logger::Level::Warning;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            level = Logger::Level::Error;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        default:
            level = Logger::Level::Info;
    }
    self->logger(level, "\e[91m[Vulkan Debug]\e[0m " + std::string(pCallbackData->pMessage));
    return VK_FALSE;
}

void Application::glfwErrorCallback(int errorCode, const char* error){
    throw std::runtime_error(std::string("GLFW encountered an error!\n ") + error + "\n");
}

template <typename T>
std::vector<T> Application::getVecProp(const boost::property_tree::ptree& pt, const boost::property_tree::ptree::key_type& key)
{
    std::vector<T> rv;
    for (auto& item : pt.get_child(key))
        rv.push_back(item.second.get_value<T>());
    return rv;
}

void Application::verifyRequired(std::string name, std::vector<std::string> available, std::vector<std::string> requested){
    logger([&name, &available, &requested](auto concat){
        concat(oz::vecToTree("Available "+name+"s" , available));
        concat(oz::vecToTree("Requested "+name+"s" , available));
    });
    for(const std::string& r : requested){
        bool found = false;
        for(const std::string& a : available) //linear search
            if(r==a){
                found = true;
                break;
            }
        if (!found){
            logger(Logger::Level::Error, name + ": " + r + " was requested but not found!");
            throw std::runtime_error(r + " was requested but not found!");
        }
    }
    logger(Logger::Level::Success, "All requested " + name + "s were found.");
}

void Application::configure(){
    boost::filesystem::path programLocation = boost::dll::program_location().parent_path();
    boost::filesystem::current_path(programLocation);
    boost::property_tree::read_json(configFilePath, staticConfigTree);
    applicationName = staticConfigTree.get<std::string>("name");
    debug = staticConfigTree.get<bool>("vkDebug");
    layerPath = staticConfigTree.get<std::string>("vkLayerPath");
    requiredVkLayers = getVecProp<std::string>(staticConfigTree, "vkLayers");
    requiredVkInstanceExtensions = getVecProp<std::string>(staticConfigTree, "vkInstanceExtensions");
    requiredVkDeviceExtensions = getVecProp<std::string>(staticConfigTree, "vkDeviceExtensions");
}

void Application::initLibs(){
    //Window Setup
    vkfw::setErrorCallback(&glfwErrorCallback);
    vkfw::init();
    vkfw::WindowHints hints {};
    hints.clientAPI = vkfw::ClientAPI::eNone;
    window = vkfw::createWindow(800, 600, applicationName.c_str(), hints);

    //Vulkan Layer Env Setup, this must be done before the enumerateInstanceLayerProperties call.
    std::string layerPathEnv = "VK_LAYER_PATH=" + layerPath;
    putenv(layerPathEnv.c_str());
    std::string layersEnv = "VK_INSTANCE_LAYERS=\"";
    for(const auto& layer : requiredVkLayers)
        layersEnv += layer + ';';
    layersEnv.pop_back(); //remove the final ";"
    layersEnv += "\"";
    putenv(layersEnv.c_str());

    //Layer Logging
    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
    std::vector<std::string> availableLayerNames = oz::map<vk::LayerProperties, std::string>(availableLayers, [](const vk::LayerProperties& l){return (std::string)l.layerName;});
    verifyRequired("Layer", availableLayerNames, requiredVkLayers);
    std::vector<const char*> finalLayers = oz::toCCPVec(requiredVkLayers);

    //Vulkan instance extensions
    std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
    std::vector<std::string> availableExtensionNames = oz::map<vk::ExtensionProperties, std::string>(availableExtensions, [](const vk::ExtensionProperties& l){return (std::string)l.extensionName;});
    std::span<const char*> glfwExtensions = vkfw::getRequiredInstanceExtensions();
    for(const char* extension : glfwExtensions)
        requiredVkInstanceExtensions.push_back(extension);
    verifyRequired("Instance Extension", availableExtensionNames, requiredVkInstanceExtensions);
    std::vector<const char*> finalExtensions = oz::toCCPVec(requiredVkInstanceExtensions);

    //Vulkan Setup
    vk::ApplicationInfo appInfo;
    appInfo = vk::ApplicationInfo();
    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = finalExtensions.size();
    createInfo.ppEnabledExtensionNames = finalExtensions.data();
    createInfo.enabledLayerCount = finalLayers.size();
    createInfo.ppEnabledLayerNames = finalLayers.data();

    vk::Result createInstanceResult = vk::createInstance(&createInfo, nullptr, &instance);
    if(createInstanceResult != vk::Result::eSuccess)
        throw std::runtime_error(std::string("VK Instance creation failed with code") + std::to_string((int)createInstanceResult));
    
    //Dynamic Dispatcher to load extensions
    dynamicloader.init(instance, vkGetInstanceProcAddr);
    
    if(debug){
        vk::DebugUtilsMessengerCreateInfoEXT dbgCreateInfo{};
        dbgCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
        dbgCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        dbgCreateInfo.pfnUserCallback = &vulkanDebugCallback;
        dbgCreateInfo.pUserData = (void*)this;
        debugMessenger = instance.createDebugUtilsMessengerEXT(dbgCreateInfo, nullptr, dynamicloader);
    }
    
    surface = vkfw::createWindowSurface(instance, window, nullptr);
    if(!(bool)surface)
        throw std::runtime_error("Failed to create surface");
}

void Application::selectPhysicalDevice(){
    //Physical Device Selection
    std::vector<vk::PhysicalDevice> allPhysicalDevices = instance.enumeratePhysicalDevices(); //All available graphics hardware
    if (allPhysicalDevices.size()==0) 
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    logger([&allPhysicalDevices](auto concat){
        std::vector<std::string> physicalDeviceNames = oz::map<vk::PhysicalDevice, std::string>(allPhysicalDevices,
            [](const vk::PhysicalDevice& d){return (std::string)d.getProperties().deviceName;});
        concat(oz::vecToTree("All Physical Graphics Hardware", physicalDeviceNames));
    });
    //Select GPUs based on desired features that are available
    for(const vk::PhysicalDevice& device : allPhysicalDevices){
        //We only allow discrte GPUs cause we're too cool for IG
        vk::PhysicalDeviceProperties deviceProps = device.getProperties();
        if(deviceProps.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
            continue;
        
        std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensions(requiredVkDeviceExtensions.begin(), requiredVkDeviceExtensions.end());
        for (const auto& extension : availableExtensions)
            requiredExtensions.erase((std::string)extension.extensionName);
        if(!requiredExtensions.empty())
            continue;

        //We need the swap chain to be good
        SwapChainInfo swapChainInfo;
        swapChainInfo.formats = device.getSurfaceFormatsKHR(surface);
        if(swapChainInfo.formats.empty())
            continue;
        swapChainInfo.presentModes = device.getSurfacePresentModesKHR(surface);
        if(swapChainInfo.formats.empty())
            continue;
        swapChainInfo.capabilities = device.getSurfaceCapabilitiesKHR(surface);

        //We have to have graphics capabilities on our GPU and surface support
        std::vector<vk::QueueFamilyProperties> queueFamProps = device.getQueueFamilyProperties();
        QueueFamilyInfo queueFamInfo;
        for(int i = 0; i < queueFamProps.size(); i++){
            if(queueFamProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
                queueFamInfo.graphicsFamily = i;
            if(device.getSurfaceSupportKHR(i, surface))
                queueFamInfo.presentationFamily = i;
        }
        if(!(queueFamInfo.graphicsFamily.has_value() && queueFamInfo.presentationFamily.has_value()))
            continue;

        PhysicalDeviceInfo physicalDeviceInfo;
        physicalDeviceInfo.physicalDevice = device;
        physicalDeviceInfo.extensionProps = availableExtensions;
        physicalDeviceInfo.queueFamilyInfo = queueFamInfo;
        physicalDeviceInfo.swapChainInfo = swapChainInfo;
        physicalDevices.push_back(std::move(physicalDeviceInfo));
    }
    if(physicalDevices.size() == 0)
        throw std::runtime_error("Failed to find GPUs that meet criteria!");
    logger([this](auto concat){
        std::vector<std::string> physicalDeviceNames = oz::map<PhysicalDeviceInfo, std::string>(physicalDevices,
            [](const PhysicalDeviceInfo& d){return (std::string)d.physicalDevice.getProperties().deviceName;});
        concat(oz::vecToTree("Suitable Physical Graphics Hardware", physicalDeviceNames));
    });
    //select first usable phsyical device
    physicalDeviceInfo = physicalDevices[0];
}

void Application::createLogicalDevice(){
    //Create a logical device
    std::vector<const char*> finalLayers = oz::toCCPVec(requiredVkLayers);
    std::vector<const char*> finalDeviceExtenstions = oz::toCCPVec(requiredVkDeviceExtensions);
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = physicalDeviceInfo.queueFamilyInfo.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &physicalDeviceInfo.queueFamilyInfo.graphicsFamilyPriority;
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = finalDeviceExtenstions.size();
    deviceCreateInfo.ppEnabledExtensionNames = finalDeviceExtenstions.data();
    deviceCreateInfo.enabledLayerCount = finalLayers.size();
    deviceCreateInfo.ppEnabledLayerNames = finalLayers.data();
    logicalDevice = physicalDeviceInfo.physicalDevice.createDevice(deviceCreateInfo);
    graphicsQueue = logicalDevice.getQueue(physicalDeviceInfo.queueFamilyInfo.graphicsFamily.value(), 0);
}

void Application::createSwapChain(){
    surfaceFormat = physicalDeviceInfo.swapChainInfo.formats[0];       //default format
    for(const auto& format : physicalDeviceInfo.swapChainInfo.formats) //find prefered format
        if(format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear){
            surfaceFormat = format;
            break;
        }
    presentMode = vk::PresentModeKHR::eFifo; //default present mode
    for(const auto& mode : physicalDeviceInfo.swapChainInfo.presentModes)
        if(mode == vk::PresentModeKHR::eMailbox){
            presentMode = mode;
            break;
        }
    vk::SurfaceCapabilitiesKHR& capabilities = physicalDeviceInfo.swapChainInfo.capabilities;
    if(capabilities.currentExtent.width != UINT32_MAX)
        swapExtent = capabilities.currentExtent;
    else{
        size_t width, height;
        window.getFramebufferSize(&width, &height);
        vk::Extent2D extent{};
        extent.width = std::clamp((uint32_t)width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp((uint32_t)height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        swapExtent = extent;
    }
    uint32_t imageCount = capabilities.minImageCount + 1;
    if(capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;
    vk::SwapchainCreateInfoKHR creationInfo;
    creationInfo.surface = surface;
    creationInfo.minImageCount = imageCount;
    creationInfo.imageFormat = surfaceFormat.format;
    creationInfo.imageColorSpace = surfaceFormat.colorSpace;
    creationInfo.imageExtent = swapExtent;
    creationInfo.imageArrayLayers = 1; //Images are flat lmao
    creationInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment; //Render directly to images in swap chain, no post processing
    QueueFamilyInfo& queueFamInf = physicalDeviceInfo.queueFamilyInfo;
    if(queueFamInf.presentationFamily == queueFamInf.graphicsFamily){
        logger(Logger::Level::Success, "Using image sharing exclusive mode (The good one)");
        creationInfo.imageSharingMode = vk::SharingMode::eExclusive; //The better one
        creationInfo.queueFamilyIndexCount = 0;
        creationInfo.pQueueFamilyIndices = nullptr;
    }else{
        logger(Logger::Level::Warning, "Using image sharing shared mode (The slow one)");
        uint32_t queueFamilyIndices[] = {queueFamInf.graphicsFamily.value(), queueFamInf.presentationFamily.value()};
        creationInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        creationInfo.queueFamilyIndexCount = 2;
        creationInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
}

Application::Application(const std::string& configFilePath_ = "config.json")
:configFilePath(configFilePath_)
{
    configure();
    initLibs();
    selectPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}

Application::~Application(){
    logicalDevice.destroy();
    instance.destroySurfaceKHR(surface);
    if(debug)
        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dynamicloader);
    instance.destroy();
    window.destroy();
    vkfw::terminate();
}

void Application::mainLoop(){
    while(!window.shouldClose()){
        vkfw::pollEvents();
    }
}

int main(int argc, char** argv){
    Application a;
    a.mainLoop();
    return 0;
}
