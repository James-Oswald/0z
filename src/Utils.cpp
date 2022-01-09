

#include<cstdint>
#include<fstream>
#include<vector>

#include "Utils.hpp"

namespace oz{
    std::vector<const char*> toCCPVec(const std::vector<std::string>& input){
        return oz::map<std::string, const char*>(input, [](const std::string& s){return s.c_str();});
    }

    std::string vecToTree(std::string root, std::vector<std::string> members){
        std::string rv = root + "\n";
        size_t size = members.size();
        for(int i = 0; i < size; i++)
            rv += (i == size - 1 ? char(192) : char(195)) + members[i] + "\n";
        return rv;
    }

    std::vector<char> readFile(const std::string& filename){
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filename);
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
}