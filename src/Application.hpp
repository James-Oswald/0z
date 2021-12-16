
#include<vkfw.hpp>

#pragma once
#include<vector>

class Application{
    private:
        static void glfwErrorCallback(int, const char*);
        void initLibs();
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
#ifdef USE_VALIDATION_LAYERS        
        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#endif
        const std::vector<const char*> requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    public:
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

        Application();
        ~Application();
        void mainLoop();
};

int main();