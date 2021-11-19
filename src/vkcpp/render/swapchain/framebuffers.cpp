#include "framebuffers.h"
#include "swapchain.h"
#include "offscreens.h"
#include "render_pass.h"

#include "device/device.h"
#include "render/image/image.h"
#include "render/image/image_depth.h"

namespace vkcpp
{
    Framebuffers::Framebuffers(const Device *device, const RenderPass *render_pass)
        : device_(device), render_pass_(render_pass)
    {
        if (render_pass->get_offscreens() != nullptr)
        {
            init_framebuffers_for_offscreen(render_pass->get_offscreens());
        }
        else
        {
            init_framebuffers(render_pass->get_swapchain());
        }
    }

    Framebuffers::~Framebuffers()
    {
        destroy_framebuffers();
    }
    inline void Framebuffers::create_framebuffer(int idx, std::vector<VkImageView> &attachments, uint32_t width, uint32_t height, uint32_t layers)
    {
        VkFramebufferCreateInfo framebufferInfo{};

        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = *render_pass_;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = layers;
#ifdef _DEBUG__
        std::cout << " framebuffer : " << width << " " << height << " " << attachments.size() << "\n";
#endif

        if (vkCreateFramebuffer(*device_, &framebufferInfo, nullptr, &handle_[idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void Framebuffers::init_framebuffers(const Swapchain *swapchain)
    {
        const std::vector<VkImageView> &image_views = swapchain->get_image_views();

        const VkExtent2D &extent = swapchain->get_properties().extent;

        uint32_t layers = swapchain->get_properties().array_layers;

        framebuffers_size_ = image_views.size();

        handle_.resize(framebuffers_size_);

        for (uint32_t i = 0; i < framebuffers_size_; i++)
        {
            std::vector<VkImageView> attachments{image_views[i]};
            if (swapchain->get_depth_size() != 0)
            {
#ifdef _DEBUG__
                std::cout << "    add depth attachment\n";
                std::cout << "    width: " << extent.width << "\n";
#endif
                attachments.push_back(swapchain->get_depth_image_view(i));
            }
            create_framebuffer(i, attachments, extent.width, extent.height, layers);
        }
    }
    void Framebuffers::init_framebuffers_for_offscreen(const Offscreens *offscreens)
    {
        const VkExtent3D &extent = offscreens->get_extent();

        framebuffers_size_ = offscreens->get_size();

        handle_.resize(framebuffers_size_);

        for (uint32_t i = 0; i < framebuffers_size_; i++)
        {
            std::vector<VkImageView> attachments;

            attachments.push_back(offscreens->get_image_view(i));

            if (offscreens->get_depth_size() != 0)
            {
                attachments.push_back(offscreens->get_depth_image_view(i));
            }

            create_framebuffer(i, attachments, extent.width, extent.height, 1);
        }
    }

    void Framebuffers::destroy_framebuffers()
    {
        for (auto &framebuffer : handle_)
        {
            if (framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(*device_, framebuffer, nullptr);
                framebuffer = VK_NULL_HANDLE;
            }
        }
        framebuffers_size_ = 0;
        handle_.resize(0);
    }
} // namespace vkcpp