
#pragma once

#include<vector>
#include<string>

namespace oz{
    template<typename T, typename U> 
    std::vector<U> map(const std::vector<T>& input, U (*applicator)(T)){
        std::vector<U> rv;
        for(const T& elm : input)
            rv.push_back(applicator(elm));
        return rv;
    }

    std::vector<const char*> toCCPVec(const std::vector<std::string>& input){
        std::vector<const char*> rv;
        for(const std::string& s : input)
            rv.push_back(s.c_str());
        return rv;
    }

    template<typename T> 
    std::string vecToString(std::vector<T> input){
        std::string rv = "[";
        for(const T& elm : input)
            rv = rv + elm + ",";
        return rv + "\b]";
    }
}