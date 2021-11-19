#ifndef VKCPP_DEVICE_INSTANCE_H
#define VKCPP_DEVICE_INSTANCE_H

#include "vulkan_header.h"

//#define NDEBUG
namespace vkcpp
{
    class PhysicalDevice;
    class Surface;
    /**
     * @brief A wrapper class for VkInstance 
     * : handling instance, gpues, debug messenger
     */
    class Instance
    {
    private:
        VkInstance handle_{VK_NULL_HANDLE};

        VkDebugUtilsMessengerEXT debug_messenger_{VK_NULL_HANDLE};

        std::vector<std::unique_ptr<PhysicalDevice>> gpus_;

#ifdef NDEBUG
        const bool enable_validation_layers_ = false;
#else
        const bool enable_validation_layers_ = true;
#endif

    public:
        static const std::vector<const char *> validation_layers_;

        Instance();

        Instance(const Instance &) = delete;

        Instance(Instance &&) = delete;

        ~Instance();

        Instance &operator=(const Instance &) = delete;

        Instance &operator=(Instance &&) = delete;

        operator const VkInstance &() const { return handle_; }

        const bool get_enable_validation_layers() const;

        /**
        * query and check support layer 
        */
        bool check_validation_layer_support();

        /**
        * get glfw instance extensions
        */
        std::vector<const char *> get_extensions();

        /**
        * debug messenger create info 
        */
        void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);

        void init_instance();

        void destroy_instance();

        void init_debug_messenger();

        void destroy_debug_messenger();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

        void destroy();

        /**
         *  @brief Quries the instance for the physical devices on the machine
         */
        void query_gpus(const Surface *surface);

        /**
         *  @brief Find the first available discrete GPU that can render to the given surface
         *  @returns A vaild PhysicalDevice  
         */
        PhysicalDevice *get_suitable_gpu(std::vector<const char *> &requested_extensions);
    }; // class Instance

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

} // namespace vkcpp

#endif