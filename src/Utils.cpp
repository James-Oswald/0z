
#include<vector>

#include "Utils.hpp"

namespace oz{
    std::vector<const char*> toCCPVec(const std::vector<std::string>& input){
        std::vector<const char*> rv;
        for(const std::string& s : input)
            rv.push_back(s.c_str());
        return rv;
    }
}