#include<vkfw.hpp>

#pragma once

#include<boost/property_tree/ptree.hpp>
#include<string>
#include<vector>

#include"Logger.hpp"

class Application{
    private:
        //cringe static glfw callback
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
        vk::Instance instance;
        std::vector<PhysicalDeviceInfo> physicalDevices; //All Physical Devices
        PhysicalDeviceInfo physicalDeviceInfo;           //A the selected physical device this app is running on.
        vk::Device logicalDevice; 
        vk::Queue graphicsQueue;
        vk::SurfaceKHR surface;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::PresentModeKHR presentMode;
        vk::Extent2D swapExtent;
    
    public:
        Application(const std::string& configFilePath);
        ~Application();
        void init();
        void mainLoop();
};

int main(int argc, char** argv);