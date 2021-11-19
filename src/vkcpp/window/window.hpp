#ifndef VKCPP_WINDOW_WINDOW_H
#define VKCPP_WINDOW_WINDOW_H

#include "vulkan_header.h"

namespace vkcpp
{
    /**
     *  @brief window interface 
     *  
     *  handling width, height, title
     */
    class Window
    {
    protected:
        uint32_t width_ = 512;
        uint32_t height_ = 512;
        std::string title_ = "";

    public:
        Window(uint32_t width, uint32_t height, std::string &title)
        {
            set_window(width, height, title);
        }
        Window() = default;
        virtual ~Window() = default;
        virtual VkSurfaceKHR create_surface(VkInstance instance) = 0;
        virtual bool should_close() = 0;
        virtual void close() = 0;
        virtual void process_events() = 0;
        std::pair<uint32_t, uint32_t> get_size()
        {
            return std::make_pair(width_, height_);
        }
        uint32_t get_width()
        {
            return width_;
        }
        uint32_t get_height()
        {
            return height_;
        }
        std::string get_title()
        {
            return title_;
        }

        virtual void set_window(uint32_t width, uint32_t height, const std::string &title)
        {
            Window::set_size(width, height);
            Window::set_title(title);
        }
        virtual void set_size(uint32_t width, uint32_t height)
        {
            width_ = width;
            height_ = height;
        }

        virtual void set_title(const std::string &title)
        {
            title_ = title;
        }
    };

} // namespace vkcpp
#endif // #ifndef VKCPP_WINDOW_