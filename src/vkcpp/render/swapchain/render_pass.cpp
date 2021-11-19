#include "render_pass.h"

#include "device/device.h"
#include "device/physical_device.h"
#include "swapchain.h"
#include "render/image/image.h"

#include "render/swapchain/offscreens.h"
#include "render/image/offscreen.h"

namespace vkcpp
{
    RenderPass::RenderPass(const Device *device, const Swapchain *swapchain)
        : device_(device), swapchain_(swapchain), offscreens_(nullptr)
    {
        init_render_pass();
    }

    RenderPass::RenderPass(const Device *device, const Offscreens *offscreens)
        : device_(device), swapchain_(nullptr), offscreens_(offscreens)
    {
        // init_render_pass();
        init_render_pass_for_offscreen();
    }

    RenderPass::~RenderPass()
    {
        destroy_rander_pass();
    }

    void RenderPass::init_render_pass()
    {
        VkFormat color_format;
        VkFormat depth_format;
        Image::getSupportedDepthFormat(device_->get_gpu(), &depth_format);
        if (swapchain_ == nullptr)
        {
            color_format = offscreens_->get_format();
            //throw std::runtime_error("failed to init renderpass for offscreen!");
        }
        else
        {
            color_format = swapchain_->get_properties().surface_format.format;
        }
#ifdef _DEBUG__
        std::cout << "      render pass color format: " << color_format << "\n";
        std::cout << "      render pass depth format: " << depth_format << "\n";
#endif

        // Describe color buffer.
        VkAttachmentDescription color_attachment{};
        color_attachment.format = color_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depth_attachment{};

        depth_attachment.format = depth_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        // Evry subpass reference one or more of the attachment, this references are themselves structs.
        // Automatically transition the attachment to this layout when the subpass is started.
        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // For multiple subpasses(ex. post-processing effects).
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        // Handle implicit automatically transition.
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        // Decide wait stage
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0; // VK_ACCESS_MEMORY_READ_BIT
        // Decide start stage and access (write)
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        std::array<VkAttachmentDescription, 2> attachments = {color_attachment, depth_attachment};

        // Describe renderpass create info
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        // Create renderpass
        if (vkCreateRenderPass(*device_, &renderPassInfo, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void RenderPass::destroy_rander_pass()
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(*device_, handle_, nullptr);
        }
    }

} // namespace vkcpp

namespace vkcpp
{
    void RenderPass::init_render_pass_for_offscreen()
    {
        if (offscreens_ == nullptr)
        {
            throw std::runtime_error("failed to init renderpass for offscreen!");
        }
        VkFormat color_format = offscreens_->get_format();
        VkFormat depth_format;
        Image::getSupportedDepthFormat(device_->get_gpu(), &depth_format);

        // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering
        std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
        // Color attachment
        attchmentDescriptions[0].format = color_format;
        attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Depth attachment
        attchmentDescriptions[1].format = depth_format;
        attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

        attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthReference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
        renderPassInfo.pAttachments = attchmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(*device_, &renderPassInfo, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass for offscreen rendering!");
        }
    }

} // namespace vkcpp
