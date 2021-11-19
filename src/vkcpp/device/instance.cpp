#include "instance.h"

#include "device/physical_device.h"
#include "device/surface.h"
#include "window/main_window.h"

namespace vkcpp
{
    Instance::Instance()
    {
        init_instance();
        init_debug_messenger();
    }
    Instance::~Instance()
    {
        destroy();
    }
    const bool Instance::get_enable_validation_layers() const
    {
        return enable_validation_layers_;
    }
    bool Instance::check_validation_layer_support()
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char *layerName : validation_layers_)
        {
            bool layer_found = false;

            for (const auto &layerProperties : available_layers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> Instance::get_extensions()
    {
        auto [window_extensions, window_count] = MainWindow::getInstance()->get_required_instance_extensions();

        std::vector<const char *> extensions(window_extensions, window_extensions + window_count);

        if (enable_validation_layers_)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
#ifdef __APPLE__
        extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

        return extensions;
    }
    void Instance::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info)
    {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debugCallback;
        create_info.pUserData = nullptr; // Optional
    }

    void Instance::init_instance()
    {
        if (enable_validation_layers_ && !check_validation_layer_support())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // Specify app info, this is optional.
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Painting";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        // Specify instance create info.
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        // Get glfw extensions + debug utils extensions.
        auto extensions = get_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        // Specify debug info for debug before create messenger and after destroy messenger
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (enable_validation_layers_)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
            create_info.ppEnabledLayerNames = validation_layers_.data();

            populate_debug_messenger_create_info(debug_create_info);
            create_info.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debug_create_info);
        }
        else
        {
            create_info.enabledLayerCount = 0;

            create_info.pNext = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &handle_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void Instance::destroy_instance()
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkDestroyInstance(handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }

    void Instance::destroy()
    {
        destroy_debug_messenger();
        destroy_instance();
    }
} // namespace vkcpp

/**
 * physical device 
 */
namespace vkcpp
{
    void Instance::query_gpus(const Surface *surface)
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(handle_, &device_count, nullptr);

        assert(!(device_count == 0) && "No physical devices were found on the system.");

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(handle_, &device_count, physical_devices.data());
        for (auto &physical_device : physical_devices)
        {
            gpus_.push_back(std::make_unique<PhysicalDevice>(this, surface, physical_device));
        }
    }
    PhysicalDevice *Instance::get_suitable_gpu(std::vector<const char *> &requested_extensions)
    {
        for (auto &gpu : gpus_)
        {
            if (gpu->get_properties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && gpu->is_device_suitable(requested_extensions))
            {
                return gpu.get();
            }
        }
        if (!gpus_.at(0)->is_device_suitable(requested_extensions))
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        // failed to find a discrete physical device, picking default GPU (first one)
        return gpus_.at(0).get();
    }

} // namespace vkcpp

/**
 * Debug 
 */
namespace vkcpp
{
    const std::vector<const char *> Instance::validation_layers_ = {
        "VK_LAYER_KHRONOS_validation"};

    void Instance::init_debug_messenger()
    {
        if (!enable_validation_layers_)
            return;

        VkDebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);

        if (CreateDebugUtilsMessengerEXT(handle_, &create_info, nullptr, &debug_messenger_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
    void Instance::destroy_debug_messenger()
    {
        if (enable_validation_layers_ && debug_messenger_ != VK_NULL_HANDLE)
        {
            DestroyDebugUtilsMessengerEXT(handle_, debug_messenger_, nullptr);
            debug_messenger_ = VK_NULL_HANDLE;
        }
    }
    VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cerr << "Warning:\n";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            std::cerr << "Info:\n";
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            std::cerr << "Error:\n";
        }
        else
        {
            return VK_FALSE;
            std::cerr << ":\n";
        }
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
} // namespace vkcpp