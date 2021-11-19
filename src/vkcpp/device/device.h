#ifndef VKCPP_DEVICE_DEVICE_H
#define VKCPP_DEVICE_DEVICE_H

#include "vulkan_header.h"
namespace vkcpp
{
    class Instance;

    class PhysicalDevice;

    class Queue;

    /**
     *  @brief A wrapper class for VkDevice
     */
    class Device
    {
    public:
        static std::mutex graphics_queue_submit_mutex_;
        const void graphics_queue_submit(const VkSubmitInfo *submit_info, int info_count, VkFence fence, const std::string &error_message = "failed to submit graphics queue") const;

        const uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    private:
        const PhysicalDevice *gpu_;

        std::unique_ptr<Queue> graphics_queue_{nullptr};

        std::unique_ptr<Queue> present_queue_{nullptr};

        std::unique_ptr<Queue> compute_queue_{nullptr};

        std::unique_ptr<Queue> transfer_queue_{nullptr};

        VkDevice handle_{VK_NULL_HANDLE};

    public:
        Device(const PhysicalDevice *gpu);

        ~Device();

        operator const VkDevice &() const
        {
            if (handle_ == VK_NULL_HANDLE)
            {
                throw std::runtime_error("failed to get device");
            }
            return handle_;
        }

        const PhysicalDevice &get_gpu() const;

        const Queue *get_graphics_queue() const;

        const Queue *get_present_queue() const;

        void init_device(const PhysicalDevice *gpu);

        void init_queues(const PhysicalDevice *gpu);

        bool check_support_blit(VkFormat swapchain_color_format) const;
    };
} // namespace vkcpp
#endif // #ifndef VKCPP_DEVICE_DEVICE_H