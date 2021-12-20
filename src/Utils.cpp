
#include<vector>

#include "Utils.hpp"

namespace oz{
    std::vector<const char*> toCCPVec(const std::vector<std::string>& input){
        return oz::map<std::string, const char*>(input, [](const std::string& s){return s.c_str();});
    }
}