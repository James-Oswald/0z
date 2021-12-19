
#include<vector>

namespace oz{
    template<typename T, typename U> 
    std::vector<U> map(std::vector<T> input, U (*applicator)(T)){
        std::vector<U> rv;
        for(const T& elm : input)
            rv.push_back(applicator(elm));
        return rv;
    }
}