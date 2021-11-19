#define STB_IMAGE_IMPLEMENTATION
#include "image2d.h"
#include "device/device.h"
#include "device/physical_device.h"
#include "render/command/command_buffers.h"

#include "utility/create.h"
namespace vkcpp
{
    Image2D::Image2D(const Device *device,
                     const CommandPool *command_pool,
                     const char *filename,
                     VkFormat format,
                     VkFilter filter,
                     VkSamplerAddressMode address_mode,
                     bool anisotropic,
                     bool mipmap,
                     bool load) : Image(device,
                                        command_pool,
                                        filter,
                                        address_mode,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                        format,
                                        1, 1, {0, 0, 1}),
                                  filename_(filename)
    {
        if (load)
        {
            init_image_2d();
            init_image_view();
            init_sampler(VK_TRUE, 0);
        }
    }
    // null texture
    Image2D::Image2D(const Device *device,
                     const CommandPool *command_pool,
                     const VkExtent3D &extent,
                     VkFormat format,
                     VkFilter filter,
                     VkSamplerAddressMode address_mode,
                     bool anisotropic,
                     bool mipmap,
                     bool load) : Image(device,
                                        command_pool,
                                        filter,
                                        address_mode,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                        format,
                                        1, 1, {0, 0, 1}),
                                  filename_(nullptr)
    {
#ifdef _DEBUG__
        std::cout << " image2D Extent : " << extent.width << " " << extent.height << "\n";
#endif
        extent_ = extent;
        if (load)
        {
            init_image_2d();
            init_image_view();
            init_sampler(VK_TRUE, 0);
        }
    }

    void Image2D::init_image_2d()
    {
        stbi_uc *pixels = nullptr;
        VkDeviceSize image_size;
        if (filename_ != nullptr)
        {
            if (format_ == VK_FORMAT_R8G8B8A8_SRGB)
            {
                pixels = stbi_load(filename_, &tex_width_, &tex_height_, &tex_channels_, STBI_rgb_alpha);
            }
            else if (format_ == VK_FORMAT_R8G8B8_SRGB)
            {
                pixels = stbi_load(filename_, &tex_width_, &tex_height_, &tex_channels_, STBI_rgb);
            }
            else
            {
                throw std::runtime_error("failed to texture load this format");
            }
            extent_ = {static_cast<uint32_t>(tex_width_), static_cast<uint32_t>(tex_height_), 1U};
        }
        else
        {
            tex_width_ = extent_.width;
            tex_height_ = extent_.height;
            tex_channels_ = 1U;
        }
        if (format_ == VK_FORMAT_R8G8B8_SRGB)
        {
            image_size = (extent_.width + extent_.width % 4) * extent_.height * 3;
        }
        else
        {
            image_size = extent_.width * extent_.height * 4;
        }

#ifdef _DEBUG__
        std::cout << tex_width_ << " " << tex_height_ << "\n";
#endif

        if (pixels == nullptr && filename_ != nullptr)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer staging_buffer;
        VkDeviceMemory staging_memory;

        create::buffer(
            device_,
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_memory);

        void *data;
        vkMapMemory(*device_, staging_memory, 0, image_size, 0, &data);

        if (filename_ != nullptr)
        {
            memcpy(data, pixels, static_cast<size_t>(image_size));
        }
        else
        {
            memset(data, 255, static_cast<size_t>(image_size));
        }

        vkUnmapMemory(*device_, staging_memory);

        if (filename_ != nullptr)
        {
            stbi_image_free(pixels);
        }

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

        CommandBuffers cmd_buffer = std::move(CommandBuffers::beginSingleTimeCmd(device_, command_pool_));

        CommandBuffers::cmdImageMemoryBarrier(
            cmd_buffer[0],
            image_,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        CommandBuffers::cmdCopyBufferToImage(
            cmd_buffer[0],
            staging_buffer,
            image_,
            extent_.width,
            extent_.height);

        CommandBuffers::cmdImageMemoryBarrier(
            cmd_buffer[0],
            image_,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            layout_,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        CommandBuffers::endSingleTimeCmd(cmd_buffer);

        vkDestroyBuffer(*device_, staging_buffer, nullptr);
        vkFreeMemory(*device_, staging_memory, nullptr);
    }

    void Image2D::init_image_view()
    {
        view_ = create::imageView(*device_,
                                  VK_IMAGE_VIEW_TYPE_2D,
                                  image_,
                                  format_,
                                  {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    }

    // image file -> texture
    void Image2D::sub_texture_image(const char *filename)
    {
        int tex_width, tex_height, tex_channels;
        stbi_uc *pixels = stbi_load(filename, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        //TODO: Add Format For RGB channel
        VkDeviceSize image_size = tex_width * tex_height * 4;
        if (format_ == VK_FORMAT_R8G8B8_SRGB)
        {
            image_size = (extent_.width + extent_.width % 4) * extent_.height * 3;
        }
        else
        {
            image_size = extent_.width * extent_.height * 4;
        }
        VkDeviceSize extent_size = tex_width_ * tex_height_ * 4;

        if (image_size != extent_size)
        {
            stbi_image_free(pixels);
            return;
        }

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer staging_buffer;
        VkDeviceMemory staging_memory;
        create::buffer(
            device_,
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_memory);

        void *data;
        vkMapMemory(*device_, staging_memory, 0, image_size, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(image_size));
        vkUnmapMemory(*device_, staging_memory);

        stbi_image_free(pixels);

        CommandBuffers cmd_buffer = std::move(CommandBuffers::beginSingleTimeCmd(device_, command_pool_));

        CommandBuffers::cmdImageMemoryBarrier(
            cmd_buffer[0],
            image_,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        CommandBuffers::cmdCopyBufferToImage(
            cmd_buffer[0],
            staging_buffer,
            image_,
            static_cast<uint32_t>(tex_width_),
            static_cast<uint32_t>(tex_height_));

        CommandBuffers::cmdImageMemoryBarrier(
            cmd_buffer[0],
            image_,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        CommandBuffers::endSingleTimeCmd(cmd_buffer);
        vkDestroyBuffer(*device_, staging_buffer, nullptr);
        vkFreeMemory(*device_, staging_memory, nullptr);
    }
} // namespace vkcpp
