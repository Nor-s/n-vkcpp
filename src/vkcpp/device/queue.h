#ifndef VKCPP_DEVICE_QUEUE_H
#define VKCPP_DEVICE_QUEUE_H

#include "vulkan_header.h"

namespace vkcpp
{
    class Device;

    /**
     *  @brief  A wrapper class for VkQueue 
     */
    class Queue
    {
    private:
        const Device *device_;

        VkQueue handle_{VK_NULL_HANDLE};

        uint32_t family_idx_{};

        uint32_t queue_idx_{};

        VkBool32 is_present_{VK_FALSE};

        VkQueueFamilyProperties properties_{};

    public:
        Queue(const Device *device, uint32_t family_idx, uint32_t queue_idx, VkBool32 is_present, VkQueueFamilyProperties properties);
        Queue(const Queue &) = default;

        Queue &operator=(const Queue &) = delete;
        Queue &operator=(Queue &&) = delete;

        operator const VkQueue &() const { return handle_; }

        const Device &get_device() const;

        const uint32_t get_family_index() const;

        const uint32_t get_index() const;

        VkQueueFamilyProperties get_properties() const;

        VkBool32 support_present() const;
    }; // class Queue
} // namespace vkcpp

#endif //#ifndef VKCPP_DEVICE_QUEUE_H