#pragma once

#include"Application.hpp"
#include"ApplicationSubsystem.hpp"
#include<vkfw.hpp>
#include<vector>


class Application{
    private:
        std::vector<ApplicationSubsystem*> subsystems;
        std::vector<const char*> requiredLayers = {"VK_LAYER_KHRONOS_validation"};
        std::vector<const char*> requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        static void glfwErrorCallback(int, const char*);
        void initSubsystems();
        void initLibs();
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
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
        void init();
        void mainLoop();
};

int main();