#ifndef VKCPP_RENDER_SWAPCHAIN_OFFSCREENS_H
#define VKCPP_RENDER_SWAPCHAIN_OFFSCREENS_H

#include "vulkan_header.h"
#include "render/image/offscreen.h"
#include "render/image/image.h"
#include "render/image/image_depth.h"

namespace vkcpp
{
    class Device;

    class ImageDepth;

    class CommandPool;

    class Offscreens
    {
    private:
        const Device *device_{nullptr};

        const CommandPool *command_pool_{nullptr};

        VkExtent3D extent_{};

        uint32_t size_{0};

        std::vector<std::unique_ptr<Offscreen>> offscreens_;

        std::vector<std::unique_ptr<ImageDepth>> depth_;

    public:
        Offscreens(const Device *device, const CommandPool *command_pool, const VkExtent3D &extent, uint32_t size);

        ~Offscreens();

        Offscreen &get_mutable_offscreen(int idx) { return *(offscreens_[idx]); }

        const VkImage &get_image(int idx) const { return offscreens_[idx]->get_image(); }

        const VkImageView &get_image_view(int idx) const { return offscreens_[idx]->get_image_view(); }

        const uint32_t get_size() const { return size_; }

        const uint32_t get_depth_size() const { return depth_.size(); }

        const VkImageView &get_depth_image_view(int idx) const { return depth_[idx]->get_image_view(); }

        const VkExtent3D &get_extent() const { return extent_; }

        const VkFormat &get_format() const { return offscreens_[0]->get_format(); }

        void init_offscreens();

        void init_depth();

        void destroy_offscreens();

    }; // class SwapChain

} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_SWAPCHAIN_OFFSCREENS_H
