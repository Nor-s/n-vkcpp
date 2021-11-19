#ifndef VKCPP_RENDER_IMAGE_IMAGE_H
#define VKCPP_RENDER_IMAGE_IMAGE_H

#include "vulkan_header.h"

namespace vkcpp
{
    class Device;
    class CommandPool;
    class Image
    {
    public:
        static void getSupportedDepthFormat(const VkPhysicalDevice physicalDevice, VkFormat *depthFormat);
        static bool hasStencilComponent(VkFormat format);

    protected:
        const Device *device_{nullptr};

        const CommandPool *command_pool_{nullptr};

        VkExtent3D extent_{};
        VkSampleCountFlagBits samples_{};
        VkImageUsageFlags usage_{};
        VkFormat format_ = VK_FORMAT_UNDEFINED;
        uint32_t mip_levels_ = 0;
        uint32_t array_layers_{};

        VkFilter filter_{};

        VkSamplerAddressMode address_mode_{};

        VkImageLayout layout_{};

        VkImage image_ = VK_NULL_HANDLE;
        VkDeviceMemory memory_ = VK_NULL_HANDLE;
        VkSampler sampler_ = VK_NULL_HANDLE;
        VkImageView view_ = VK_NULL_HANDLE;
        std::vector<char> is_moved_;

    public:
        Image() = default;
        Image(const Image &) = delete;
        explicit Image(const Device *device,
                       const CommandPool *command_pool,
                       VkFilter filter,
                       VkSamplerAddressMode addressMode,
                       VkSampleCountFlagBits samples,
                       VkImageLayout layout,
                       VkImageUsageFlags usage,
                       VkFormat format,
                       uint32_t mipLevels,
                       uint32_t arrayLayers,
                       const VkExtent3D &extent);
        virtual ~Image();

        Image(Image &&a) = default;

        Image &operator=(Image &&a) = default;

        const VkImageView &get_image_view() const
        {
            return view_;
        }
        const VkImage &get_image() const
        {
            return image_;
        }
        const VkSampler &get_sampler() const { return sampler_; }

        const VkFormat &get_format() const { return format_; }

        const VkExtent3D &get_extent() const { return extent_; }

        const std::pair<uint32_t, uint32_t> get_size() const { return {extent_.width, extent_.height}; }

        void init_sampler(VkBool32 anisotropic, uint32_t mip_levels);

        void destroy_image();

        /**
         * Must same width and height
         * Maybe problem : read and write at the same time
         */
        void sub_image(VkImage host_src_image, VkExtent3D src_extent, const VkFormat &swapchain_color_format);
    };

} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_IMAGE_IMAGE_H