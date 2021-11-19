#include "image_depth.h"
#include "device/device.h"
#include "device/physical_device.h"
#include "render/command/command_buffers.h"

#include "utility/create.h"
namespace vkcpp
{

    ImageDepth::ImageDepth(const Device *device,
                           const CommandPool *command_pool,
                           VkExtent3D extent,
                           VkSampleCountFlagBits samples) : Image(device,
                                                                  command_pool,
                                                                  VK_FILTER_LINEAR,
                                                                  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                                                                  samples,
                                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, //| VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                  VK_FORMAT_UNDEFINED,
                                                                  1, 1, extent)
    {
        VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;

        getSupportedDepthFormat(device_->get_gpu(), &format_);
        if (hasStencilComponent(format_))
        {
            //    aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
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

        view_ = create::imageView(
            *device_,
            VK_IMAGE_VIEW_TYPE_2D,
            image_,
            format_,
            {aspect_mask, 0, 1, 0, 1});
#ifdef _DEBUG__
        std::cout << "   depth: " << extent_.width << " " << extent_.height << "\n";
        std::cout << "    depth format: " << format_ << "\n";
#endif

        //   init_depth_sampler();
        //init_sampler(false, 1);
    }

    void ImageDepth::init_depth_sampler()
    {
        // Create sampler to sample from the attachment in the fragment shader
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = filter_;
        samplerInfo.minFilter = filter_;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = address_mode_;
        samplerInfo.addressModeV = address_mode_;
        samplerInfo.addressModeW = address_mode_;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        if (vkCreateSampler(*device_, &samplerInfo, nullptr, &sampler_))
        {
            std::cout << "failed to create sampler for offscrreen\n";
            return;
        }
    }

} // namespace vkcpp