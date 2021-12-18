
//Base logger

#include<vkfw.hpp>

#pragma once

#include<string>
#include<functional>

class Logger{
    private:
        std::function<void(const std::string&)> logCallback;
        static void defaultLogCallback(const std::string&);
    public:
        Logger();
        Logger(std::function<void(const std::string&)>);
        enum class Level{Info, Warning, Error}
        void operator()(Level, std::string);
        void operator()(Level, std::function<void(std::function<void(std::string)>)>);
        void operator()(std::string);
        void operator()(std::function<void(std::function<void(std::string)>)>);
};


