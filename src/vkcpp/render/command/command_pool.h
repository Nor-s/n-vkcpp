#ifndef VKCPP_COMMAND_COMMAND_POOL_H
#define VKCPP_COMMAND_COMMAND_POOL_H

#include "vulkan_header.h"
namespace vkcpp
{
    class Device;

    class Queue;

    class CommandPool
    {
    private:
        const Device *device_;

        const Queue *queue_;

        VkCommandPool handle_{VK_NULL_HANDLE};

    public:
        CommandPool(const Device *device, const Queue *queue, VkCommandPoolCreateFlags flags);

        ~CommandPool();

        operator const VkCommandPool &() const { return handle_; }

        const uint32_t get_queue_family_idx() const;

        const Queue &get_queue() const;

        void init_command_pool(VkCommandPoolCreateFlags flags, uint32_t queue_family_index);

        void destroy_command_pool();

        void alloc_buffers(std::vector<VkCommandBuffer> &buffer, uint32_t size, VkCommandBufferLevel level) const;
    }; // class CommandPool
} // namespace vkcpp

#endif // #ifndef VKCPP_COMMAND_COMMAND_POOL_H