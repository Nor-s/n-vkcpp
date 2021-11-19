#include "physical_device.h"

#include "instance.h"
#include "surface.h"

namespace vkcpp
{
    PhysicalDevice::PhysicalDevice(const Instance *instance, const Surface *surface, VkPhysicalDevice physical_device)
        : instance_(instance), surface_(surface), handle_(physical_device)
    {
        vkGetPhysicalDeviceFeatures(physical_device, &supported_features_);
        vkGetPhysicalDeviceProperties(physical_device, &properties_);

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        queue_family_properties_.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties_.data());
    }

    QueueFamilyIndices PhysicalDevice::find_queue_families()
    {
        QueueFamilyIndices indices;
        int i = 0;
        for (const auto &queue_family : queue_family_properties_)
        {
            // Check for graphics support.
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.supported_queues |= VK_QUEUE_GRAPHICS_BIT;
                indices.graphics_family = i;
            }

            // Check for present support.
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(handle_, i, *surface_, &present_support);
            if (present_support && queue_family.queueCount > 0)
            {
                indices.present_family = i;
            }

            // Check for compute support.
            if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                indices.supported_queues |= VK_QUEUE_COMPUTE_BIT;
                indices.compute_family = i;
            }

            // Check for transfer support.
            if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices.supported_queues |= VK_QUEUE_TRANSFER_BIT;
                indices.transfer_family = i;
            }

            if (indices.is_complete())
            {
                return indices;
            }

            i++;
        }
        return indices;
    }
    bool PhysicalDevice::check_device_extension_support(std::vector<const char *> &requested_extensions)
    {
        uint32_t extension_count;
        vkEnumerateDeviceExtensionProperties(handle_, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(handle_, nullptr, &extension_count, available_extensions.data());

        std::set<std::string> required_extensions(requested_extensions.begin(), requested_extensions.end());

        for (const auto &extension : available_extensions)
        {
            required_extensions.erase(extension.extensionName);
        }

        return required_extensions.empty();
    }
    SwapchainSupportDetails PhysicalDevice::query_swapchain_support() const
    {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle_, *surface_, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(handle_, *surface_, &format_count, nullptr);

        if (format_count != 0)
        {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(handle_, *surface_, &format_count, details.formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(handle_, *surface_, &present_mode_count, nullptr);

        if (present_mode_count != 0)
        {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(handle_, *surface_, &present_mode_count, details.present_modes.data());
        }

        return details;
    }
    bool PhysicalDevice::is_device_suitable(std::vector<const char *> &requested_extensions)
    {
        extensions_ = requested_extensions;
        queue_family_indices_ = find_queue_families();
        bool extensions_supported = check_device_extension_support(requested_extensions);
        bool swapchain_adequate = false;
        if (extensions_supported)
        {
            auto swapchain_support = query_swapchain_support();
            swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
        }

        return queue_family_indices_.is_graphics_and_present() && extensions_supported && swapchain_adequate && supported_features_.samplerAnisotropy;
    }

} // namespace vkcpp
