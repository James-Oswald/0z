
#pragma once

#include<vector>
#include"Application.hpp"

class ApplicationSubsystem{
    private:
        Application* application;
        std::vector<const char*> requiredLayers;
        std::vector<const char*> requiredDeviceExtensions;
    public:
        ApplicationSubsystem();
        void setApplication(Application*);
};