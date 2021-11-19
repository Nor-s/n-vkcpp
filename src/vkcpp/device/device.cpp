#include "device.h"

#include "instance.h"
#include "surface.h"
#include "physical_device.h"
#include "queue.h"

/**
 * query
*/
namespace vkcpp
{
    std::mutex Device::graphics_queue_submit_mutex_;

    const void Device::graphics_queue_submit(const VkSubmitInfo *submit_info, int info_count, VkFence fence, const std::string &error_message) const
    {
        graphics_queue_submit_mutex_.lock();
        // Submit to the queue
        if (vkQueueSubmit(*graphics_queue_, info_count, submit_info, fence) != VK_SUCCESS)
        {
            throw std::runtime_error(error_message);
        }

        graphics_queue_submit_mutex_.unlock();
    }
    const uint32_t Device::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const
    {
        const VkPhysicalDeviceMemoryProperties &mem_properties = gpu_->get_memory_properties();

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

} // namespace vkcpp

namespace vkcpp
{
    Device::Device(const PhysicalDevice *gpu)
        : gpu_(gpu)
    {
        init_device(gpu_);
        init_queues(gpu_);
    }
    Device::~Device()
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkDestroyDevice(handle_, nullptr);
        }
    }
    const PhysicalDevice &Device::get_gpu() const
    {
        return *gpu_;
    };

    const Queue *Device::get_graphics_queue() const
    {
        return graphics_queue_.get();
    }

    const Queue *Device::get_present_queue() const
    {
        return present_queue_.get();
    }
    void Device::init_device(const PhysicalDevice *gpu)
    {
        const QueueFamilyIndices &indices = gpu->get_queue_family_indices();
        const Instance &instance = gpu->get_instance();

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};
        if (indices.compute_family.has_value())
        {
            unique_queue_families.insert(indices.compute_family.value());
        }
        if (indices.transfer_family.has_value())
        {
            unique_queue_families.insert(indices.transfer_family.value());
        }

        // Specify queues to be created
        float queue_priority = 1.0f;
        for (uint32_t queue_family : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        // Specify device features
        const VkPhysicalDeviceFeatures &device_features = gpu->get_features();

        // Specify device extensions
        const std::vector<const char *> &device_extensions = gpu->get_extensions();

        // Specify Device create info
        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();

        if (instance.get_enable_validation_layers())
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(instance.validation_layers_.size());
            create_info.ppEnabledLayerNames = instance.validation_layers_.data();
        }
        else
        {
            create_info.enabledLayerCount = 0;
        }

        // Creat the logicald device
        if (vkCreateDevice(*gpu, &create_info, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }
    }
    void Device::init_queues(const PhysicalDevice *gpu)
    {
        const QueueFamilyIndices &indices = gpu->get_queue_family_indices();

        graphics_queue_ = std::make_unique<Queue>(this, indices.graphics_family.value(), 0, false, gpu->get_queue_family_properties()[indices.graphics_family.value()]);
        present_queue_ = std::make_unique<Queue>(this, indices.present_family.value(), 0, true, gpu->get_queue_family_properties()[indices.present_family.value()]);
        if (indices.compute_family.has_value())
        {
            graphics_queue_ = std::make_unique<Queue>(this, indices.compute_family.value(), 0, false, gpu->get_queue_family_properties()[indices.compute_family.value()]);
        }
        if (indices.transfer_family.has_value())
        {
            graphics_queue_ = std::make_unique<Queue>(this, indices.transfer_family.value(), 0, false, gpu->get_queue_family_properties()[indices.transfer_family.value()]);
        }
    }

    bool Device::check_support_blit(VkFormat swapchain_color_format) const
    {
        bool supports_blit = true;
        // Check blit support for source and destination
        VkFormatProperties format_props;

        // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
        vkGetPhysicalDeviceFormatProperties(*gpu_, swapchain_color_format, &format_props);
        if (!(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
        {
            std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
            supports_blit = false;
        }
        //    supports_blit = false;
        /*
        // Check if the device supports blitting to linear images
        vkGetPhysicalDeviceFormatProperties(*gpu_, VK_FORMAT_R8G8B8A8_UNORM, &format_props);
        if (!(format_props.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
        {
            std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
            supports_blit = false;
        }
        */
        return supports_blit;
    }

} // namespace vkcpp