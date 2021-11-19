#include "image.h"
#include "device/device.h"
#include "device/physical_device.h"
#include "render/command/command_buffers.h"

#include "utility/create.h"

namespace vkcpp
{
    static constexpr float MIN_ANISOTROPY = 16.0f;

    Image::Image(const Device *device,
                 const CommandPool *command_pool,
                 VkFilter filter,
                 VkSamplerAddressMode addressMode,
                 VkSampleCountFlagBits samples,
                 VkImageLayout layout,
                 VkImageUsageFlags usage,
                 VkFormat format,
                 uint32_t mipLevels,
                 uint32_t arrayLayers,
                 const VkExtent3D &extent) : device_(device),
                                             command_pool_(command_pool),
                                             extent_(extent),
                                             samples_(samples),
                                             usage_(usage),
                                             format_(format),
                                             mip_levels_(mipLevels),
                                             array_layers_(arrayLayers),
                                             filter_(filter),
                                             address_mode_(addressMode),
                                             layout_(layout)
    {
        is_moved_.push_back('e');
    }

    Image::~Image()
    {
        if (is_moved_.size() > 0)
        {
#ifdef _DEBUG__
            std::cout << "dtor image\n";
#endif
            destroy_image();
        }
    }
    void Image::init_sampler(VkBool32 anisotropic, uint32_t mip_levels)
    {

        VkSamplerCreateInfo sampler_info{};

        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = filter_; // or VK_FILTER_NEAREST
        sampler_info.minFilter = filter_; // or VK_FILTER_NEAREST

        sampler_info.addressModeU = address_mode_;
        sampler_info.addressModeV = address_mode_;
        sampler_info.addressModeW = address_mode_;

        const VkPhysicalDeviceProperties &properties = device_->get_gpu().get_properties();
        sampler_info.anisotropyEnable = anisotropic;
        sampler_info.maxAnisotropy = (anisotropic) ? std::min(MIN_ANISOTROPY, properties.limits.maxSamplerAnisotropy) : 1.0f;

        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;

        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = static_cast<float>(mip_levels);

        if (vkCreateSampler(*device_, &sampler_info, nullptr, &sampler_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
    void Image::destroy_image()
    {
        if (sampler_ != VK_NULL_HANDLE)
        {
            vkDestroySampler(*device_, sampler_, nullptr);
            sampler_ = VK_NULL_HANDLE;
        }
        if (view_ != VK_NULL_HANDLE)
        {
            vkDestroyImageView(*device_, view_, nullptr);
            view_ = VK_NULL_HANDLE;
        }
        if (image_ != VK_NULL_HANDLE)
        {
            vkDestroyImage(*device_, image_, nullptr);
            image_ = VK_NULL_HANDLE;
        }
        if (memory_ != VK_NULL_HANDLE)
        {
            vkFreeMemory(*device_, memory_, nullptr);
            memory_ = VK_NULL_HANDLE;
        }
    }
    bool Image::hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT;
    }
    void Image::getSupportedDepthFormat(const VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
    {
        // Since all depth formats may be optional, we need to find a suitable depth format to use
        // Start with the highest precision packed format
        std::vector<VkFormat> depthFormats = {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM};

        for (auto &format : depthFormats)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
            // Format must support depth stencil attachment for optimal tiling
            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                *depthFormat = format;
                return;
            }
        }
        throw std::runtime_error("failed to find depth format!");
    }

    // TODO: move to image2D, only work image2D <-> image2D
    void Image::sub_image(VkImage host_src_image, VkExtent3D src_extent, const VkFormat &color_format)
    {
        if (!(extent_.width == src_extent.width && extent_.height == src_extent.height))
        {
            return;
        }
        bool supportsBlit = device_->check_support_blit(color_format);

        // Do the actual blit from the swapchain image to our host visible destination image
        vkcpp::CommandBuffers copy_cmd = std::move(vkcpp::CommandBuffers::beginSingleTimeCmd(device_, command_pool_));
        // Transition destination image to transfer destination layout
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            image_,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        // Transition swapchain image from present to transfer source layout
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            host_src_image,
            0, //VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        vkcpp::CommandBuffers::cmdCopyImage(
            copy_cmd[0],
            supportsBlit,
            extent_,
            {0, 0, 1},
            {0, 0, 1},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u},
            host_src_image, image_);

        // Transition destination image to general layout, which is the required layout for mapping the image memory later on
        CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            image_,
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
            host_src_image,
            VK_ACCESS_TRANSFER_READ_BIT, //VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     //VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, //VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, //VK_PIPELINE_STAGE_TRANSFER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        copy_cmd.flush_command_buffer(0);
    }

} // namespace vkcpp
