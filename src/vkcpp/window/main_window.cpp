#include "main_window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace vkcpp
{
    MainWindow::MainWindow()
    {
        init_window(512, 512, title_);
    }
    MainWindow::~MainWindow()
    {
        if (handle_)
        {
            destroy_window();
        }
    }
    VkSurfaceKHR MainWindow::create_surface(VkInstance instance)
    {
        if (instance == VK_NULL_HANDLE || !handle_)
        {
            throw std::runtime_error("failed to access instance or GLFWwindow*!");
        }
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance, handle_, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
        return surface;
    }
    bool MainWindow::should_close()
    {
        return glfwWindowShouldClose(handle_);
    }
    void MainWindow::close()
    {
        glfwSetWindowShouldClose(handle_, GLFW_TRUE);
    }
    void MainWindow::process_events()
    {
        glfwPollEvents();
    }
    // TODO process input
    void MainWindow::process_input()
    {
    }

    void MainWindow::init_window(uint32_t width, uint32_t height, std::string &title)
    {
        set_window(width, height, title);
        if (!handle_)
        {
            init_glfw();
        }
    }

    void MainWindow::init_glfw()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("GLFW couldn't be initialized.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        handle_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
        if (!handle_)
        {
            glfwTerminate();
            throw std::runtime_error("GLFW failed to create window");
        }
    }

    void MainWindow::destroy_window()
    {
        glfwDestroyWindow(handle_);
        handle_ = nullptr;
        glfwTerminate();
    }

    void MainWindow::wait_events()
    {
        glfwWaitEvents();
    }
    void MainWindow::set_factor()
    {
        int frame_width, frame_height;
        int window_width, window_height;
        glfwGetFramebufferSize(handle_, &frame_width, &frame_height);
        glfwGetWindowSize(handle_, &window_width, &window_height);
        factor_ = frame_width / window_width;
    }
    float MainWindow::get_factor()
    {
        set_factor();
        return factor_;
    }

    std::pair<const char **, uint32_t> MainWindow::get_required_instance_extensions() const
    {
        uint32_t glfw_extension_count;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        return std::make_pair(glfw_extensions, glfw_extension_count);
    }

    std::pair<int, int> MainWindow::get_framebuffer_size()
    {
        std::pair<int, int> size;
        glfwGetFramebufferSize(handle_, &size.first, &size.second);
        Window::set_size(size.first, size.second);
        return size;
    }

    void MainWindow::set_window(uint32_t width, uint32_t height, const std::string &title)
    {
        MainWindow::set_size(width, height);
        MainWindow::set_title(title);
        std::cout << "width: " << get_width() << " "
                  << " height" << get_height() << "\n";
    }

    void MainWindow::set_size(uint32_t width, uint32_t height)
    {
        Window::set_size(width, height);
        glfwSetWindowSize(handle_, width_, height_);
    }

    void MainWindow::set_title(const std::string &title)
    {
        Window::set_title(title);
        glfwSetWindowTitle(handle_, title_.c_str());
    }

    void MainWindow::set_user_pointer(void *pointer)
    {
        glfwSetWindowUserPointer(handle_, pointer);
    }

    void MainWindow::set_framebuffer_size_callback(void (*fp)(GLFWwindow *window, int width, int height))
    {
        glfwSetFramebufferSizeCallback(handle_, fp);
    }

    void MainWindow::set_drop_callback(void (*fp)(GLFWwindow *window, int count, const char **paths))
    {
        glfwSetDropCallback(handle_, fp);
    }
    void MainWindow::set_scroll_callback(void (*fp)(GLFWwindow *window, double xoffset, double yoffset))
    {
        glfwSetScrollCallback(handle_, fp);
    }
    void MainWindow::set_mouse_button_callback(void (*fp)(GLFWwindow *window, int button, int action, int modes))
    {
        glfwSetMouseButtonCallback(handle_, fp);
    }

} // namespace vkcpp