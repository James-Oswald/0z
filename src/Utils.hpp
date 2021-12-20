
#pragma once

#include<vector>
#include<string>

namespace oz{
    template<typename T> std::string vecToString(std::vector<T> input);
    template<typename T, typename U> std::vector<U> map(const std::vector<T>& input, U (*applicator)(T));
    template<typename T, typename U> std::vector<U> smap(const std::vector<T>& input, U (*applicator)(const T&));
    std::vector<const char*> toCCPVec(const std::vector<std::string>& input);
}

#include "Utils.tpp"