
#pragma once

#include<vector>

namespace oz{
    template<typename T, typename U> std::vector<U> map(std::vector<T>, U (*)(T));
}