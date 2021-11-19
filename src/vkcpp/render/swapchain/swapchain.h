#ifndef VKCPP_RENDER_SWAPCHAIN_SWAPCHAIN_H
#define VKCPP_RENDER_SWAPCHAIN_SWAPCHAIN_H

#include "vulkan_header.h"
#include "render/image/image.h"
#include "render/image/image_depth.h"

namespace vkcpp
{
    class Device;
    class Surface;

    struct SwapchainProperties
    {
        VkSwapchainKHR old_swapchain{};

        uint32_t image_count{0};

        VkExtent2D extent{};

        VkSurfaceFormatKHR surface_format{};

        uint32_t array_layers{};

        VkImageUsageFlags image_usage{};

        VkSurfaceTransformFlagBitsKHR pre_transform{};

        VkCompositeAlphaFlagBitsKHR composite_alpha{};

        VkPresentModeKHR present_mode{};

        VkSharingMode sharing_mode{};

        VkBool32 clipped{};
    };

    class Swapchain
    {
    private:
        const Device *device_;

        const Surface *surface_;

        VkSwapchainKHR handle_{VK_NULL_HANDLE};

        std::vector<VkImage> images_;

        std::vector<VkImageView> image_views_;

        std::vector<std::unique_ptr<ImageDepth>> depth_;

        SwapchainProperties properties_{};

    public:
        Swapchain(const Device *device, const Surface *surface);

        ~Swapchain();

        operator const VkSwapchainKHR &() const { return handle_; }

        const std::vector<VkImage> &get_images() const { return images_; }

        const std::vector<VkImageView> &get_image_views() const { return image_views_; };

        const uint32_t get_depth_size() const { return depth_.size(); }

        const VkImageView &get_depth_image_view(int idx) const { return depth_[idx]->get_image_view(); }

        const SwapchainProperties &get_properties() const { return properties_; }

        /**
         * @brief Choose VK_PRESENT_MODE_MAILBOX_KHR and VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
         */
        VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);

        /**
         *  @brief If device surface can't support VK_PRESENT_MODE_MAILBOX_KHR, then choose VK_PRESENT_MODE_FIFO_KHR
         */
        VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);

        /**
         *  @brief Choose capabilities.currentExtent or clamped glfw framebuffer size
         */
        VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities);

        /**
         *  Fill properties_ and Create swapchain
         *  compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
         *  cliped = false
         *  oldSwapchain = VK_NULL_HANDLE
         */
        void init_swapchain(const Device *device, const Surface *surface);

        void destroy_swapchain();

        void init_images();

        void init_image_views();

        void init_depth();
    }; // class SwapChain

} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_SWAPCHAIN_SWAPCHAIN_H
