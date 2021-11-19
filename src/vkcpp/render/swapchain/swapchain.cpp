#include "swapchain.h"

#include "window/main_window.h"
#include "device/surface.h"
#include "device/physical_device.h"
#include "device/device.h"
#include "render/image/image.h"
#include "render/image/image_depth.h"
#include "utility/create.h"

namespace vkcpp
{
    Swapchain::Swapchain(const Device *device, const Surface *surface)
        : device_(device), surface_(surface)
    {
        init_swapchain(device_, surface_);
        init_images();
        init_image_views();
        init_depth();
    }

    Swapchain::~Swapchain()
    {
        destroy_swapchain();
    }

    VkSurfaceFormatKHR Swapchain::choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats)
    {
        for (const auto &available_format : available_formats)
        {
            if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return available_format;
            }
        }

        return available_formats[0];
    }
    VkPresentModeKHR Swapchain::choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes)
    {
        for (const auto &available_present_mode : available_present_modes)
        {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return available_present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D Swapchain::choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            auto [width, height] = MainWindow::getInstance()->get_framebuffer_size();

            VkExtent2D actual_extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actual_extent;
        }
    }

    void Swapchain::init_swapchain(const Device *device, const Surface *surface)
    {
        auto swapchain_support = device->get_gpu().query_swapchain_support();

        // Choose device surfac support detail
        properties_.surface_format = choose_swapchain_surface_format(swapchain_support.formats);
        properties_.present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
        properties_.extent = choose_swapchain_extent(swapchain_support.capabilities);

        // Decide minimum image count to have in the swap chain, +1 is spare image
        properties_.image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 && properties_.image_count > swapchain_support.capabilities.maxImageCount)
        {
            properties_.image_count = swapchain_support.capabilities.maxImageCount;
        }

        // Fill swap chain create info struct
        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = *surface;

        create_info.minImageCount = properties_.image_count;
        create_info.imageFormat = properties_.surface_format.format;
        create_info.imageColorSpace = properties_.surface_format.colorSpace;
        create_info.imageExtent = properties_.extent;
        create_info.imageArrayLayers = properties_.array_layers = 1; // 2D image: 1, 3D image: >1
        create_info.imageUsage = properties_.image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // Decide image sharing mode
        const QueueFamilyIndices &indices = device->get_gpu().get_queue_family_indices();
        uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

        if (indices.graphics_family != indices.present_family)
        {
            create_info.imageSharingMode = properties_.sharing_mode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            create_info.imageSharingMode = properties_.sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        }

        create_info.preTransform = properties_.pre_transform = swapchain_support.capabilities.currentTransform;
        create_info.compositeAlpha = properties_.composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = properties_.present_mode;
        create_info.clipped = properties_.clipped = VK_FALSE;

        create_info.oldSwapchain = properties_.old_swapchain = VK_NULL_HANDLE;

        // Create Swapchain
        if (vkCreateSwapchainKHR(*device, &create_info, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }
    }
    void Swapchain::init_depth()
    {
        int size = static_cast<int>(image_views_.size());
        for (int i = 0; i < size; i++)
        {
            VkExtent3D extent{properties_.extent.width, properties_.extent.height, 1};
            depth_.push_back(std::make_unique<ImageDepth>(device_, nullptr, extent));
        }
    }

    void Swapchain::destroy_swapchain()
    {
        int size = depth_.size();
        for (int i = 0; i < size; i++)
        {
            depth_[i].reset();
        }
        depth_.resize(0);
        for (auto &image_view : image_views_)
        {
            vkDestroyImageView(*device_, image_view, nullptr);
            image_view = VK_NULL_HANDLE;
        }
        image_views_.resize(0);
        if (handle_ != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(*device_, handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }

    void Swapchain::init_images()
    {
        vkGetSwapchainImagesKHR(*device_, handle_, &properties_.image_count, nullptr);
        images_.resize(properties_.image_count);
        vkGetSwapchainImagesKHR(*device_, handle_, &properties_.image_count, images_.data());
    }

    void Swapchain::init_image_views()
    {
        image_views_.resize(images_.size());

        for (size_t i = 0; i < images_.size(); i++)
        {
            image_views_[i] = create::imageView(*device_,
                                                VK_IMAGE_VIEW_TYPE_2D,
                                                images_[i],
                                                properties_.surface_format.format,
                                                {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        }
    }
} // namespace vkcpp

/*
// Take a screenshot from the current swapchain image
// This is done using a blit from the swapchain image to a linear image whose memory content is then saved as a ppm image
// Getting the image date directly from a swapchain image wouldn't work as they're usually stored in an implementation dependent optimal tiling format
// Note: This requires the swapchain images to be created with the VK_IMAGE_USAGE_TRANSFER_SRC_BIT flag (see VulkanSwapChain::create)
void PaintingApplication::save_screen_shot(const char *filename, VkFormat swapchain_color_format, VkImage src_image, VkExtent3D extent)
{
    const vkcpp::Device *device = device_.get();
    const vkcpp::CommandPool *command_pool = command_pool_.get();
    bool supportsBlit = device_->check_support_blit(swapchain_color_format);

    // Source for the copy is the last rendered swapchain image
    // VkImage srcImage = swapChain.images[currentBuffer];

    // Create the linear tiled destination image to copy to and to read the memory from
    // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
    VkImage dst_image;
    // Create memory to back up the image
    VkDeviceMemory dst_image_memory{nullptr};
    vkcpp::create::image(device,
                         VK_IMAGE_TYPE_2D,
                         VK_FORMAT_R8G8B8A8_UNORM,
                         extent,
                         VK_IMAGE_TILING_LINEAR,
                         VK_SAMPLE_COUNT_1_BIT,
                         VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // Memory must be host visible to copy from
                         dst_image,
                         dst_image_memory);

    // Do the actual blit from the swapchain image to our host visible destination image
    vkcpp::CommandBuffers copy_cmd = std::move(vkcpp::CommandBuffers::beginSingleTimeCmd(device, command_pool));
    // begin
    // Transition destination image to transfer destination layout
    vkcpp::CommandBuffers::cmdImageMemoryBarrier(
        copy_cmd[0],
        dst_image,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    // Transition swapchain image from present to transfer source layout
    vkcpp::CommandBuffers::cmdImageMemoryBarrier(
        copy_cmd[0],
        src_image,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vkcpp::CommandBuffers::cmdCopyImage(
        copy_cmd[0],
        supportsBlit,
        extent,
        {VK_IMAGE_ASPECT_COLOR_BIT, 1}, {VK_IMAGE_ASPECT_COLOR_BIT, 1},
        src_image, dst_image);

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    vkcpp::CommandBuffers::cmdImageMemoryBarrier(
        copy_cmd[0],
        dst_image,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    // Transition back the swap chain image after the blit is done
    vkcpp::CommandBuffers::cmdImageMemoryBarrier(
        copy_cmd[0],
        src_image,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    copy_cmd.flush_command_buffer(0);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(*device, dst_image, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char *data;
    vkMapMemory(*device, dst_image_memory, 0, VK_WHOLE_SIZE, 0, (void **)&data);
    data += subResourceLayout.offset;

    std::ofstream file(filename, std::ios::out | std::ios::binary);

    // ppm header
    file << "P6\n"
         << extent.width << "\n"
         << extent.height << "\n"
         << 255 << "\n";

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    bool colorSwizzle = false;
    // Check if source is BGR
    // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
    if (!supportsBlit)
    {
        std::vector<VkFormat> formatsBGR = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
        colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), swapchain_color_format) != formatsBGR.end());
    }

    // ppm binary pixel data
    for (uint32_t y = 0; y < extent.height; y++)
    {
        unsigned int *row = (unsigned int *)data;
        for (uint32_t x = 0; x < extent.width; x++)
        {
            if (colorSwizzle)
            {
                file.write((char *)row + 2, 1);
                file.write((char *)row + 1, 1);
                file.write((char *)row, 1);
            }
            else
            {
                file.write((char *)row, 3);
            }
            row++;
        }
        data += subResourceLayout.rowPitch;
    }
    file.close();

    std::cout << "Screenshot saved to disk" << std::endl;

    // Clean up resources
    vkUnmapMemory(*device, dst_image_memory);
    vkFreeMemory(*device, dst_image_memory, nullptr);
    vkDestroyImage(*device, dst_image, nullptr);
}
*/