#include "command_pool.h"

#include "device/device.h"
#include "device/physical_device.h"
#include "device/queue.h"

namespace vkcpp
{
    CommandPool::CommandPool(const Device *device, const Queue *queue, VkCommandPoolCreateFlags flags)
        : device_(device), queue_(queue)
    {
        init_command_pool(flags, queue->get_family_index());
    }

    CommandPool::~CommandPool()
    {
        destroy_command_pool();
    }

    const uint32_t CommandPool::get_queue_family_idx() const
    {
        return queue_->get_family_index();
    }

    const Queue &CommandPool::get_queue() const
    {
        return *queue_;
    }

    void CommandPool::init_command_pool(VkCommandPoolCreateFlags flags, uint32_t queue_family_idx)
    {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = queue_family_idx;
        pool_info.flags = flags;

        if (vkCreateCommandPool(*device_, &pool_info, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }
    void CommandPool::destroy_command_pool()
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(*device_, handle_, nullptr);
        }
    }
    void CommandPool::alloc_buffers(std::vector<VkCommandBuffer> &buffer, uint32_t size, VkCommandBufferLevel level) const
    {
        buffer.resize(size);

        VkCommandBufferAllocateInfo alloc_info{};

        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = *this;
        alloc_info.level = level;
        alloc_info.commandBufferCount = size;

        if (vkAllocateCommandBuffers(*device_, &alloc_info, buffer.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
} // namespace vkcpp
