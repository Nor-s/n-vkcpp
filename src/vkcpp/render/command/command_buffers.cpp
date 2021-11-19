#include "command_buffers.h"

#include "device/device.h"
#include "command_pool.h"
#include "render/render_stage.h"
#include "render/pipeline/pipeline.hpp"
#include "render/swapchain/framebuffers.h"
#include "render/swapchain/render_pass.h"
#include "render/swapchain/swapchain.h"
#include "device/queue.h"

// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000
namespace vkcpp
{
    CommandBuffers CommandBuffers::beginSingleTimeCmd(const Device *device, const CommandPool *command_pool)
    {
        CommandBuffers cmd_buffer(device, command_pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        cmd_buffer.begin_command_buffer(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        return cmd_buffer;
    }

    void CommandBuffers::endSingleTimeCmd(CommandBuffers &cmd_buffer)
    {
        const VkQueue &graphics_queue = *(cmd_buffer.get_device().get_graphics_queue());
        cmd_buffer.end_command_buffer(0);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &(cmd_buffer.get_command_buffers(0));

        cmd_buffer.get_device().graphics_queue_submit(&submit_info, 1, VK_NULL_HANDLE, "failed to submit in end single time!");

        vkQueueWaitIdle(graphics_queue);

        cmd_buffer.free_command_buffers();
    }
    void CommandBuffers::cmdCopyBuffer(VkCommandBuffer cmd_buffer,
                                       VkBuffer src_buffer,
                                       VkBuffer dst_buffer,
                                       VkDeviceSize size)
    {
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = size;
        vkCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);
    }

    void CommandBuffers::cmdSingleCopyBuffer(const Device *device,
                                             const CommandPool *command_pool,
                                             VkBuffer src_buffer,
                                             VkBuffer dst_buffer,
                                             VkDeviceSize size)
    {
        CommandBuffers cmd_buffer = std::move(beginSingleTimeCmd(device, command_pool));

        cmdCopyBuffer(cmd_buffer[0],
                      src_buffer,
                      dst_buffer,
                      size);

        endSingleTimeCmd(cmd_buffer);
    }
    void CommandBuffers::cmdBufferMemoryBarrier(VkCommandBuffer cmd_buffer,
                                                VkBuffer buffer,
                                                VkDeviceSize offset,
                                                VkDeviceSize size,
                                                VkAccessFlags src_access_mask,
                                                VkAccessFlags dst_access_mask,
                                                VkPipelineStageFlags src_stage_mask,
                                                VkPipelineStageFlags dst_stage_mask)
    {
        VkBufferMemoryBarrier buffer_memory_barrier{};
        buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        buffer_memory_barrier.srcAccessMask = src_access_mask;
        buffer_memory_barrier.dstAccessMask = dst_access_mask;
        buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buffer_memory_barrier.buffer = buffer;
        buffer_memory_barrier.offset = offset;
        buffer_memory_barrier.size = size;

        vkCmdPipelineBarrier(
            cmd_buffer,
            src_stage_mask,
            dst_stage_mask,
            0,
            0, nullptr,
            1, &buffer_memory_barrier,
            0, nullptr);
    }
    void CommandBuffers::cmdImageMemoryBarrier(VkCommandBuffer cmd_buffer,
                                               VkImage image,
                                               VkAccessFlags src_access_mask,
                                               VkAccessFlags dst_access_mask,
                                               VkImageLayout old_image_layout,
                                               VkImageLayout new_image_layout,
                                               VkPipelineStageFlags src_stage_mask,
                                               VkPipelineStageFlags dst_stage_mask,
                                               VkImageSubresourceRange subresource_range)
    {
        VkImageMemoryBarrier image_memory_barrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.srcAccessMask = src_access_mask;
        image_memory_barrier.dstAccessMask = dst_access_mask;
        image_memory_barrier.oldLayout = old_image_layout;
        image_memory_barrier.newLayout = new_image_layout;
        image_memory_barrier.image = image;
        image_memory_barrier.subresourceRange = subresource_range;
        image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // For queue family ownership
        image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // For queue family ownership

        vkCmdPipelineBarrier(
            cmd_buffer,
            src_stage_mask,
            dst_stage_mask,
            0,
            0, nullptr,
            0, nullptr,
            1, &image_memory_barrier);
    }
    void CommandBuffers::cmdCopyImageToBuffer(VkCommandBuffer cmd_buffer,
                                              VkBuffer buffer,
                                              VkImage image,
                                              VkOffset3D offset,
                                              VkExtent3D extent)
    {

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = offset;
        region.imageExtent = extent;

        vkCmdCopyImageToBuffer(
            cmd_buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            buffer,
            1,
            &region);
    }
    void CommandBuffers::cmdCopyBufferToImage(VkCommandBuffer cmd_buffer,
                                              VkBuffer buffer,
                                              VkImage image,
                                              uint32_t width,
                                              uint32_t height)
    {

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1};

        vkCmdCopyBufferToImage(
            cmd_buffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);
    }

    void CommandBuffers::cmdCopyImage(VkCommandBuffer cmd_buffer,
                                      bool supports_blit,
                                      const VkExtent3D &extent,
                                      const VkOffset3D &src_offset,
                                      const VkOffset3D &dst_offset,
                                      const VkImageSubresourceLayers &src_subresource,
                                      const VkImageSubresourceLayers &dst_subresource,
                                      VkImage src_image,
                                      VkImage dst_image)
    {
#ifdef _DEBUG__
        std::cout << " cmd copy image : " << extent.width << " " << extent.height << " " << extent.depth << "\n";
#endif
        // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
        if (supports_blit)
        {
            // Define the region to blit (we will blit the whole swapchain image)
            VkOffset3D blit_size;
            blit_size.x = extent.width;
            blit_size.y = extent.height;
            blit_size.z = extent.depth;

            VkImageBlit blig_region{};
            blig_region.srcSubresource = src_subresource;
            blig_region.srcOffsets[0] = src_offset;
            blig_region.srcOffsets[1] = blit_size;

            blig_region.dstSubresource = dst_subresource;
            blig_region.dstOffsets[0] = dst_offset;
            blig_region.dstOffsets[1] = blit_size;

            // Issue the blit command
            vkCmdBlitImage(
                cmd_buffer,
                src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blig_region,
                VK_FILTER_NEAREST);
        }
        else
        {
            // Otherwise use image copy (requires us to manually flip components)
            VkImageCopy copy_region{};
            copy_region.extent = extent;
            copy_region.srcOffset = src_offset;
            copy_region.dstOffset = dst_offset;
            copy_region.srcSubresource = src_subresource;
            copy_region.dstSubresource = dst_subresource;

            // Issue the copy command
            vkCmdCopyImage(
                cmd_buffer,
                src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copy_region);
        }
    }
} // namespace vkcpp

namespace vkcpp
{
    CommandBuffers::CommandBuffers(const Device *device, const CommandPool *command_pool, uint32_t size, VkCommandBufferLevel level)
        : device_(device), command_pool_(command_pool), level_(level)
    {
        handle_.resize(size);
        init_command_buffers();
    }

    CommandBuffers::CommandBuffers(CommandBuffers &&a)
        : device_(a.device_), command_pool_(a.command_pool_), level_(std::move(a.level_)), handle_(std::move(a.handle_))
    {
        a.handle_.resize(0);
    }

    CommandBuffers::~CommandBuffers()
    {
        free_command_buffers();
    }

    CommandBuffers &CommandBuffers::operator=(CommandBuffers &&a)
    {
        if (this != &a)
        {
            device_ = a.device_;
            command_pool_ = a.command_pool_;
            level_ = a.level_;
            handle_ = std::move(a.handle_);
            a.handle_.resize(0);
        }
        return *this;
    }

    void CommandBuffers::init_command_buffers()
    {
        if (command_pool_ == nullptr)
        {
            throw std::runtime_error("failed to allocate command buffers : command pool is nullptr!");
        }
        command_pool_->alloc_buffers(handle_, static_cast<uint32_t>(handle_.size()), level_);
    }

    void CommandBuffers::free_command_buffers()
    {
        if (handle_.size() > 0)
        {
            vkFreeCommandBuffers(*device_, *command_pool_, static_cast<uint32_t>(handle_.size()), handle_.data());
            handle_.resize(0);
        }
    }

    void CommandBuffers::begin_command_buffer(int command_buffer_idx, VkCommandBufferUsageFlags flags)
    {
        VkCommandBufferBeginInfo begin_info{};

        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = flags;
        begin_info.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(handle_[command_buffer_idx], &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    }

    void CommandBuffers::begin_render_pass(int command_buffer_idx, const RenderStage *render_stage)
    {
        render_stage->begin_render_pass(handle_[command_buffer_idx], command_buffer_idx);
    }

    void CommandBuffers::bind_pipeline(int command_buffer_idx, const Pipeline *pipeline)
    {
        pipeline->bind_pipeline(handle_[command_buffer_idx]);
    }

    void CommandBuffers::end_render_pass(int command_buffer_idx, const RenderStage *render_stage)
    {
        render_stage->end_render_pass(handle_[command_buffer_idx], command_buffer_idx);
    }

    void CommandBuffers::end_command_buffer(int command_buffer_idx)
    {
        if (vkEndCommandBuffer(handle_[command_buffer_idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void CommandBuffers::flush_command_buffer(int command_buffer_idx)
    {
        end_command_buffer(0);

        auto &command_buffer = handle_[command_buffer_idx];
        if (command_buffer == VK_NULL_HANDLE)
        {
            return;
        }

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = 0;

        VkFence fence;

        if (vkCreateFence(*device_, &fence_info, nullptr, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fence for a flush command buffer!");
        }
        device_->graphics_queue_submit(&submit_info, 1, fence, "failed to submit in flush_command_buffer!");
        // Wait for the fence to signal that command buffer has finished executing
        if (vkWaitForFences(*device_, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to wait fence for flush command buffer!");
        }
        vkDestroyFence(*device_, fence, nullptr);
        // free buffer
    }

} // namespace vkcpp
