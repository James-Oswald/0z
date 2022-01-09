
#pragma once

#include<cstdint>
#include<vector>
#include<string>

namespace oz{
    //Real utils
    template<typename T, typename U> std::vector<U> map(const std::vector<T>& input, U (*applicator)(T));

    //String formatting and conversion utils
    template<typename T> std::string vecToString(std::vector<T> input);
    std::vector<const char*> toCCPVec(const std::vector<std::string>& input);
    std::string vecToTree(std::string root, std::vector<std::string> members);

    //file utils
    std::vector<char> readFile(const std::string& filename);
}
#include "Utils.tpp"