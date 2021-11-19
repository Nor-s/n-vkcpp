#include "utility.h"

namespace vkcpp
{
    void VK_CHECK_RESULT(VkBool32 a)
    {
        if (a != VK_SUCCESS)
        {
            return;
        }
    }
    float getRandFloat(float lo, float hi)
    {
        return lo + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (hi - lo)));
    }
    float getProbablity()
    {
        return getRandFloat(0.0f, 1.0f);
    }
    std::vector<char> readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        std::cout << filename << " \n";

        if (!file.is_open())
        {
            throw std::runtime_error("utility.cpp readFile : failed to open file!");
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();

        return buffer;
    }
} // namespace vkcpp