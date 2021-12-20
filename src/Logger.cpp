
#include<cstdio>
#include"Logger.hpp"


void Logger::defaultLogCallback(const std::string& value){
    printf("%s",value.c_str());
}

Logger::Logger()
:Logger(&defaultLogCallback)
{}

Logger::Logger(std::function<void(const std::string&)> callback)
:logCallback(callback)
{}

void Logger::operator()(Level level, std::string message){
    std::string logMessage = "\n";
    switch(level){
        case Level::Success:
            logMessage += "\e[1;92m[Success]\e[0m";
            break;
        case Level::Error:
            logMessage += "\e[1;31m[Error]\e[0m";
            break;
        case Level::Warning:
            logMessage += "\e[1;33m[Warning]\e[0m";
            break;
        case Level::Info:
        default:
            logMessage += "\e[1;34m[Info]\e[0m";
    }
    std::string cleanedMessage = std::string(message.begin(), message.end() - (message[message.size()-1] == '\n' ? 1 : 0));  
    logMessage += (cleanedMessage.find('\n') == std::string::npos ? " " : "\n") + cleanedMessage;
    logCallback(logMessage);
}

void Logger::operator()(std::string message){
    this->operator()(Level::Info, message);
}

//Aggregation implementation.
void Logger::operator()(Level level, std::function<void(std::function<void(std::string)>)> callback){
    std::string logMessage = "";
    std::function<void(std::string)> aggregator = [&logMessage](std::string messagePart){logMessage += messagePart;};
    callback(aggregator);
    this->operator()(level, logMessage);
}

void Logger::operator()(std::function<void(std::function<void(std::string)>)> callback){
    this->operator()(Level::Info, callback);
}

