#include "offscreen.h"
#include "device/device.h"
#include "device/queue.h"
#include "device/physical_device.h"
#include "render/command/command_buffers.h"
#include "render/command/command_pool.h"

#include "utility/create.h"
namespace vkcpp
{
    Offscreen::Offscreen(const Device *device,
                         const CommandPool *command_pool,
                         const VkExtent3D &extent,
                         VkSampleCountFlagBits samples) : Image(device,
                                                                nullptr,
                                                                VK_FILTER_LINEAR,
                                                                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                                samples,
                                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                VK_FORMAT_R8G8B8A8_SRGB,
                                                                1, 1, extent)
    {
        //TODO: test code
        uniq_command_pool_ = std::make_unique<vkcpp::CommandPool>(device_, device_->get_graphics_queue(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        command_pool_ = uniq_command_pool_.get();

        image_size_ = extent.width * extent.height * 4;

        create::image(
            device_,
            VK_IMAGE_TYPE_2D,
            format_,
            extent_,
            VK_IMAGE_TILING_OPTIMAL,
            samples_,
            usage_,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            image_,
            memory_);

        view_ = create::imageView(
            *device_,
            VK_IMAGE_VIEW_TYPE_2D,
            image_,
            format_,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        create::buffer(
            device_,
            image_size_,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            staging_buffer_,
            staging_memory_);

        init_sampler(false, 1);
    }
    Offscreen::~Offscreen()
    {
        vkQueueWaitIdle(*device_->get_graphics_queue());
        if (is_mapping)
        {
            unmap_memory();
        }
        vkDestroyBuffer(*device_, staging_buffer_, nullptr);
        vkFreeMemory(*device_, staging_memory_, nullptr);
        uniq_command_pool_.reset();
    }

    //TODO : fix hard coding "supportsBlit = false"
    const char *Offscreen::map_image_memory()
    {
        if (is_mapping)
        {
            unmap_memory();
        }
        is_mapping = true;
        const vkcpp::Device *device = device_;
        //   VkFormat format = get_format();
        //  bool supportsBlit = false; //= device_->check_support_blit(object->get_);
        VkImage src_image = get_image();
        //  VkExtent3D extent = extent_;

        // Do the actual blit from the swapchain image to our host visible destination image
        vkcpp::CommandBuffers copy_cmd = std::move(vkcpp::CommandBuffers::beginSingleTimeCmd(device, command_pool_));

        // Transition destination image to transfer destination layout
        vkcpp::CommandBuffers::cmdBufferMemoryBarrier(
            copy_cmd[0],
            staging_buffer_,
            0,
            image_size_,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT);
        // Transition swapchain image from present to transfer source layout
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            src_image,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        vkcpp::CommandBuffers::cmdCopyImageToBuffer(
            copy_cmd[0],
            staging_buffer_,
            src_image,
            {0, 0, 0},
            extent_);
        vkcpp::CommandBuffers::cmdBufferMemoryBarrier(
            copy_cmd[0],
            staging_buffer_,
            0,
            image_size_,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT);

        // Transition back the swap chain image after the blit is done
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            src_image,
            VK_ACCESS_TRANSFER_READ_BIT, //VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     //VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, //VK_PIPELINE_STAGE_TRANSFER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        copy_cmd.flush_command_buffer(0);

        // Map image memory so we can start copying from it
        const char *data;
        vkMapMemory(*device, staging_memory_, 0, VK_WHOLE_SIZE, 0, (void **)&data);
        return data;
    }
    void Offscreen::unmap_memory()
    {
        vkUnmapMemory(*device_, staging_memory_);
        is_mapping = false;
    }
    void Offscreen::screen_to_image(const CommandPool *command_pool, VkImage host_dst_image, const VkExtent3D &src_extent, const VkOffset3D &src_offset, const VkFormat &color_format)
    {
        if (!(extent_.width >= src_offset.x + src_extent.width && extent_.height >= src_offset.y + src_extent.height))
        {
            return;
        }
        bool supportsBlit = true; //device_->check_support_blit(color_format);

        // Do the actual blit from the swapchain image to our host visible destination image
        vkcpp::CommandBuffers copy_cmd = std::move(vkcpp::CommandBuffers::beginSingleTimeCmd(device_, command_pool));
        // Transition destination image to transfer destination layout
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            host_dst_image,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        // Transition swapchain image from present to transfer source layout
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            image_,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        vkcpp::CommandBuffers::cmdCopyImage(
            copy_cmd[0],
            supportsBlit,
            extent_,
            src_offset,
            src_offset,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            image_, host_dst_image);

        // Transition destination image to general layout, which is the required layout for mapping the image memory later on
        CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            host_dst_image,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        // Transition back the swap chain image after the blit is done
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            image_,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        copy_cmd.flush_command_buffer(0);
    }

} // namespace vkcpp