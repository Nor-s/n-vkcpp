#include "render_stage.h"

#include "device/device.h"
#include "device/physical_device.h"
#include "render/swapchain/swapchain.h"
#include "render/swapchain/offscreens.h"
#include "render/image/image.h"

namespace vkcpp
{
    RenderStage::RenderStage(const Device *device, const Swapchain *swapchain)
        : device_(device), swapchain_(swapchain), offscreens_(nullptr)
    {
        color_format_ = swapchain_->get_properties().surface_format.format;
        init_render_stage();
    }

    RenderStage::RenderStage(const Device *device, const Offscreens *offscreens)
        : device_(device), swapchain_(nullptr), offscreens_(offscreens)
    {
        color_format_ = offscreens_->get_format();
        init_render_stage();
    }

    RenderStage::~RenderStage()
    {
        destroy();
    }

    void RenderStage::init_render_stage()
    {
        Image::getSupportedDepthFormat(device_->get_gpu(), &depth_format_);

        VkExtent2D extent;
        VkClearValue clear_color{};
        clear_color.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values_.emplace_back(clear_color);
        VkClearValue clear_depth{};
        clear_depth.depthStencil = {1.0f, 0};
        clear_values_.emplace_back(clear_depth);

        if (swapchain_ == nullptr)
        {
            render_pass_ = std::make_unique<RenderPass>(device_, offscreens_);
            framebuffers_ = std::make_unique<Framebuffers>(device_, render_pass_.get());
            extent.width = offscreens_->get_extent().width;
            extent.height = offscreens_->get_extent().height;
        }
        else
        {
            render_pass_ = std::make_unique<RenderPass>(device_, swapchain_);
            framebuffers_ = std::make_unique<Framebuffers>(device_, render_pass_.get());
            extent = swapchain_->get_properties().extent;
        }

        render_area_.offset.x = 0.0;
        render_area_.offset.y = 0.0;
        render_area_.extent = extent;
#ifndef NDEBUG
        std::cout << render_area_.extent.width << " " << render_area_.extent.height << "\n";
#endif
    }

    void RenderStage::destroy()
    {
        framebuffers_.reset();
        render_pass_.reset();
        clear_values_.clear();
    }

    void RenderStage::begin_render_pass(const VkCommandBuffer &command_buffer, int framebuffer_idx) const
    {
        VkRect2D render_area = get_render_area();
        VkViewport viewport{};
        viewport.x = static_cast<float>(render_area.offset.x);
        viewport.y = static_cast<float>(render_area.offset.y);
        viewport.width = static_cast<float>(render_area.extent.width);
        viewport.height = static_cast<float>(render_area.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = render_area.offset;
        scissor.extent = render_area.extent;
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // Start renderpass
        VkRenderPassBeginInfo render_pass_info{};

        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = get_render_pass();
        render_pass_info.framebuffer = get_framebuffer(framebuffer_idx);
        render_pass_info.renderArea = get_render_area();
        render_pass_info.clearValueCount = clear_values_.size(); // get_clear_values().size();
        render_pass_info.pClearValues = clear_values_.data();    // get_clear_values().data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderStage::end_render_pass(const VkCommandBuffer &command_buffer, int framebuffer_idx) const
    {
        vkCmdEndRenderPass(command_buffer);
    }

} // namespace vkcpp