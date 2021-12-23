
//Base logger
#pragma once

#include<ctime>
#include<string>
#include<functional>

class Logger{
    private:
        time_t startTime;
        std::function<void(const std::string&)> logCallback;
        static void defaultLogCallback(const std::string&);
    public:
        enum class Level{Starting, Success, Info, Warning, Error};
        Logger();
        Logger(std::function<void(const std::string&)>);
        void operator()(Level, std::string);
        //void operator()(Level, const std::string&);
        void operator()(Level, std::function<void(std::function<void(std::string)>)>);
        void operator()(std::string);
        //void operator()(const std::string&);
        void operator()(std::function<void(std::function<void(std::string)>)>);
};


