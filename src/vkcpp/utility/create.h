#ifndef VKCPP_UTILITY_CREATE_H
#define VKCPP_UTILITY_CREATE_H

#include "vulkan_header.h"

namespace vkcpp
{
    class Device;
    namespace create
    {
        VkImageView imageView(VkDevice device,
                              VkImageViewType view_type,
                              VkImage image,
                              VkFormat format,
                              VkImageSubresourceRange sub_resource_range);

        void image(const Device *device,
                   VkImageType image_type,
                   VkFormat format,
                   VkExtent3D extent,
                   VkImageTiling tiling,
                   VkSampleCountFlagBits samples,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage &image,
                   VkDeviceMemory &imageMemory);

        void buffer(const Device *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory);
        /**
         *  create shaderModule using spirv code
         */
        VkShaderModule shaderModule(const Device *device, std::string &filename);
    } // namespace create
} // namespace vkcpp

#endif