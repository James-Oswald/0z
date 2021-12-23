#pragma once

#include<master.hpp>

#include<vulkan/vulkan.h>
#include<vkfw.hpp>
#include<boost/property_tree/ptree.hpp>
#include<string>
#include<vector>

#include"Logger.hpp"

class Application{
    private:
        //cringe static glfw callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
        static void glfwErrorCallback(int errorCode, const char* errorMessage);    //callback for glfw errors

        //helpers
        void verifyRequired(std::string name, std::vector<std::string> available, std::vector<std::string> requested);
        template<typename T> static std::vector<T> getVecProp(const boost::property_tree::ptree& tree, const boost::property_tree::ptree::key_type& vecPropName);
        
        //Main subsystems
        Logger logger;

        //Config properties
        std::string configFilePath;
        boost::property_tree::ptree staticConfigTree;
        std::string applicationName;
        bool debug;
        std::string layerPath;
        std::vector<std::string> requiredVkLayers;            //Required vulkan instance layers
        std::vector<std::string> requiredVkInstanceExtensions; 
        std::vector<std::string> requiredVkDeviceExtensions;  //Required logical device extensions

        //Chunk Helpers
        void configure();
        void initLibs();
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createGraphicsPipeline();
    protected:
        struct QueueFamilyInfo{
            float graphicsFamilyPriority = 1;
            float presentationFamilyPriority = 1;
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentationFamily; 
        };
        struct SwapChainInfo{
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> presentModes;
        };
        struct PhysicalDeviceInfo{ //Holds information about physical devices
            vk::PhysicalDevice physicalDevice;
            QueueFamilyInfo queueFamilyInfo;
            SwapChainInfo swapChainInfo;
            std::vector<vk::ExtensionProperties> extensionProps;
        };

        vkfw::Window window;
        vk::DispatchLoaderDynamic dynamicloader;
        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;
        std::vector<PhysicalDeviceInfo> physicalDevices; //All Physical Devices
        PhysicalDeviceInfo physicalDeviceInfo;           //A the selected physical device this app is running on.
        vk::Device logicalDevice; 
        vk::Queue graphicsQueue;
        vk::SurfaceKHR surface; 
        vk::SurfaceFormatKHR surfaceFormat;

        //Swapchain
        vk::SwapchainKHR swapchain; 
        vk::PresentModeKHR presentMode;
        vk::Extent2D swapExtent;
        std::vector<vk::Image> swapchainImages; //vk::images are handles to images in the swapchain 
        std::vector<vk::ImageView> swapchainImageViews; //handles to into the images + metadata, what we actually write to.
        //std::vector<vk::Framebuffer> swapchainFrameBuffers; 
    
    public:
        Application(const std::string& configFilePath);
        ~Application();
        void init();
        void mainLoop();
};

int main(int argc, char** argv);