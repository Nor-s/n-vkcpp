#include "queue.h"

#include "device.h"

namespace vkcpp
{
    Queue::Queue(const Device *device, uint32_t family_idx, uint32_t queue_idx, VkBool32 is_present, VkQueueFamilyProperties properties)
        : device_(device), family_idx_(family_idx), queue_idx_(queue_idx), is_present_(is_present), properties_(properties)
    {
        vkGetDeviceQueue(*device, family_idx, queue_idx, &handle_);
    }

    const Device &Queue::get_device() const
    {
        return *device_;
    }

    const uint32_t Queue::get_family_index() const
    {
        return family_idx_;
    }

    const uint32_t Queue::get_index() const
    {
        return queue_idx_;
    }

    VkQueueFamilyProperties Queue::get_properties() const
    {
        return properties_;
    }

    VkBool32 Queue::support_present() const
    {
        return is_present_;
    }

} // namespace vkcpp
