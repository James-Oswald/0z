
namespace oz{
    template<typename T> 
    std::string vecToString(std::vector<T> input){
        std::string rv = "[";
        for(const T& elm : input)
            rv = rv + elm + ",";
        return rv + "\b]";
    }

    template<typename T, typename U> 
    std::vector<U> map(const std::vector<T>& input, U (*applicator)(const T&)){
        std::vector<U> rv;
        for(const T& elm : input)
            rv.push_back(applicator(elm));
        return rv;
    }
}

