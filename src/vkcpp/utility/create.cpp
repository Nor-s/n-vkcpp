#include "create.h"
#include "device/device.h"
#include "utility.h"
namespace vkcpp
{
    namespace create
    {
        VkImageView imageView(VkDevice device,
                              VkImageViewType view_type,
                              VkImage image,
                              VkFormat format,
                              VkImageSubresourceRange sub_resource_range)
        {
            // Specify createinfo.
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = image;
            view_info.viewType = view_type; // dimension of texture.
            view_info.format = format;

            // components field allows to swizzle the color channels around.
            view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            // Describes what the image's purpose is and which part of the image should be accessed.
            view_info.subresourceRange = sub_resource_range;

            VkImageView image_view;
            if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create texture image view!");
            }

            return image_view;
        }

        void image(const Device *device,
                   VkImageType image_type,
                   VkFormat format,
                   VkExtent3D extent,
                   VkImageTiling tiling,
                   VkSampleCountFlagBits samples,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage &image,
                   VkDeviceMemory &imageMemory)
        {
            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = image_type;
            image_info.extent = extent;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.format = format;
            image_info.tiling = tiling;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_info.usage = usage;
            image_info.samples = samples;
            image_info.flags = 0; // Optional
            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(*device, &image_info, nullptr, &image) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements mem_requirements;
            vkGetImageMemoryRequirements(*device, image, &mem_requirements);

            VkMemoryAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_requirements.size;
            alloc_info.memoryTypeIndex = device->find_memory_type(mem_requirements.memoryTypeBits, properties);

            if (vkAllocateMemory(*device, &alloc_info, nullptr, &imageMemory) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate image memory!");
            }

            if (vkBindImageMemory(*device, image, imageMemory, 0) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to bind image memory!");
            }
        }

        void buffer(const Device *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &memory)
        {
            VkBufferCreateInfo buffer_info{};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = size;
            buffer_info.usage = usage;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_info.flags = 0; // for sparse buffer memory

            if (vkCreateBuffer(*device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements mem_requirements;
            vkGetBufferMemoryRequirements(*device, buffer, &mem_requirements);

            VkMemoryAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_requirements.size;
            alloc_info.memoryTypeIndex = device->find_memory_type(mem_requirements.memoryTypeBits, properties);

            if (vkAllocateMemory(*device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(*device, buffer, memory, 0);
        }

        VkShaderModule shaderModule(const Device *device, std::string &filename)
        {
            VkShaderModule shader_module;
            std::vector<char> code = readFile(filename);
            VkShaderModuleCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = code.size();
            create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

            if (vkCreateShaderModule(*device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create shader module!");
            }
            return shader_module;
        }
    } // namespace create
} // namespace vkcpp