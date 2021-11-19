#ifndef VKCPP_RENDER_SWAPCHAIN_FRAMEBUFFERS_H
#define VKCPP_RENDER_SWAPCHAIN_FRAMEBUFFERS_H

#include "vulkan_header.h"

namespace vkcpp
{
    class Device;
    class Swapchain;
    class RenderPass;
    class Image;
    class Offscreens;

    class Framebuffers
    {
    private:
        const Device *device_;

        const RenderPass *render_pass_;

        std::vector<VkFramebuffer> handle_;

        uint32_t framebuffers_size_{0};

    public:
        Framebuffers(const Device *device, const RenderPass *render_pass);

        ~Framebuffers();

        const uint32_t &get_framebuffers_size() const { return framebuffers_size_; }

        const RenderPass &get_render_pass() const { return *render_pass_; };

        const std::vector<VkFramebuffer> &get_framebuffers() const { return handle_; };

        void create_framebuffer(int idx, std::vector<VkImageView> &attachments, uint32_t width, uint32_t height, uint32_t layers);

        void init_framebuffers(const Swapchain *swapchain);

        void init_framebuffers_for_offscreen(const Offscreens *offscreens);

        void destroy_framebuffers();
    };
} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_SWAPCHAIN_FRAMEBUFFER_H