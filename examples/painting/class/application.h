#ifndef PAINTING_APPLICATION_H
#define PAINTING_APPLICATION_H

#include "vkcpp.h"
#include "brush.h"
#include "picture.h"

namespace painting
{
    class PaintingApplication
    {
        const uint32_t MAX_FRAMES_IN_FLIGHT = 3;

    public:
        static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
        static void dropCallback(GLFWwindow *window, int count, const char **paths);

        std::vector<const char *> device_extensions_ = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
            ,
            "VK_KHR_portability_subset"
#endif
        };

        PaintingApplication() = default;
        void run(uint32_t width = 512, uint32_t height = 512, std::string title = "painting");

    private:
        std::unique_ptr<vkcpp::Instance> instance_{nullptr};
        std::unique_ptr<vkcpp::Surface> surface_{nullptr};
        std::unique_ptr<vkcpp::Device> device_{nullptr};
        std::unique_ptr<vkcpp::Swapchain> swapchain_{nullptr};
        std::unique_ptr<vkcpp::RenderStage> render_stage_{nullptr};
        std::unique_ptr<vkcpp::CommandPool> command_pool_{nullptr};
        std::unique_ptr<vkcpp::CommandBuffers> command_buffers_{nullptr};

        std::vector<std::unique_ptr<vkcpp::Object2D>> object_;
        std::unique_ptr<Picture> picture_{nullptr};

        std::vector<VkSemaphore> image_available_semaphores_;
        std::vector<VkSemaphore> render_finished_semaphores_;
        std::vector<VkFence> in_flight_fences_;
        std::vector<VkFence> images_in_flight_;

        size_t current_frame_ = 0;

        bool framebuffer_resized_ = false;

        std::vector<bool> is_command_buffer_updated_;

        void init_window(uint32_t width, uint32_t height, std::string title);
        void init_device();
        void init_render();
        void init_synobj();
        void record_command_buffers();
        void record_command_buffer(int idx);
        void update_uniform_buffer(uint32_t obj_idx, uint32_t ubo_idx);
        void draw_frame();
        void main_loop();
        void cleanup();
        void reset_command_buffers_update_flag();
        void cleanup_swapchain();
        void recreate_swapchain();
    };
} // namespace painting
#endif // #ifndef PAINTING_APPLICATION_H