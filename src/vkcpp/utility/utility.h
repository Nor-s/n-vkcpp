#ifndef VKCPP_UTILITY_UTILITY_H
#define VKCPP_UTILITY_UTILITY_H

#include "vulkan_header.h"

namespace vkcpp
{
    void VK_CHECK_RESULT(VkBool32 a);
    float getRandFloat(float lo, float hi);
    float getProbablity();
    std::vector<char> readFile(const std::string &filename);
} // namespace vkcpp

#endif // #ifndef VKCPP_UTILITY_UTILITY_HPP