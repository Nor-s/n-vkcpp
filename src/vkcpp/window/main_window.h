#ifndef VKCPP_WINDOW_MAIN_WINDOW_H
#define VKCPP_WINDOW_MAIN_WINDOW_H

#include "window.hpp"
#include "pattern/singleton.hpp"
#include "std_header.h"

struct GLFWwindow;
namespace vkcpp
{
    class MainWindow : public Window, public Singleton<MainWindow>
    {
        inline static const bool is_instanced_ = initInstance();

    private:
        GLFWwindow *handle_ = nullptr;

    public:
        MainWindow();
        MainWindow(const MainWindow &) = delete;
        MainWindow &operator=(const MainWindow &) = delete;
        virtual ~MainWindow();
        virtual VkSurfaceKHR create_surface(VkInstance instance) override;
        virtual bool should_close() override;
        virtual void close() override;
        virtual void process_events() override;
        void process_input();
        void init_window(uint32_t width, uint32_t height, std::string &title);
        void init_glfw();
        void destroy_window();
        void wait_events();

        std::pair<const char **, uint32_t> get_required_instance_extensions() const;
        std::pair<int, int> get_framebuffer_size();

        virtual void set_window(uint32_t width, uint32_t height, const std::string &title) override;
        virtual void set_size(uint32_t width, uint32_t height) override;
        virtual void set_title(const std::string &title) override;
        void set_user_pointer(void *pointer);
        void set_framebuffer_size_callback(void (*fp)(GLFWwindow *window, int width, int height));
        void set_drop_callback(void (*fp)(GLFWwindow *window, int count, const char **paths));
        void set_scroll_callback(void (*fp)(GLFWwindow *window, double xoffset, double yoffset));
    };
} // namespace vkcpp
#endif // #ifndef VKCPP_MAIN_WINDOW_H