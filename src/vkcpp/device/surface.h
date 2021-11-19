#ifndef VKCPP_DEVICE_SURFACE_H
#define VKCPP_DEVICE_SURFACE_H

#include "vulkan_header.h"

namespace vkcpp
{
    class Instance;
    class Surface
    {
    private:
        const Instance *instance_;

        VkSurfaceKHR handle_{VK_NULL_HANDLE};

    public:
        Surface() = default;

        Surface(const Instance *instance, VkSurfaceKHR surface);

        ~Surface();

        operator const VkSurfaceKHR &() const { return handle_; }

        const Instance &get_instance() const;

        void destroy_surface();
    };
} // namespace vkcpp

#endif
