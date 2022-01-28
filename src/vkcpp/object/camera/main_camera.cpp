#include "main_camera.h"
#include "window/main_window.h"

namespace vkcpp
{
    MainCamera::MainCamera()
    {

        auto [width, height] = static_cast<std::pair<float, float>>(MainWindow::getInstance()->get_framebuffer_size());
        width_ = width;
        height_ = height;

        update_view_to_look_at(
            {0.0f, 0.0f, 1000.0f},
            {0.0f, 0.0f, 2001.0f},
            {0.0f, 1.0f, 0.0f});

        update_proj_to_perspective(
            glm::radians(100.0f),
            width / height,
            {0.1f, 2000.0f});
        // update_proj_to_ortho({-width / 2.0f, width / 2.0f}, {-height / 2.0f, height / 2.0f}, {-100.0f, 100.0f});
        // update_proj_to_ortho({0.0f, width}, {0.0f, height}, {-100.0f, 100.0f});
    }
    void MainCamera::update()
    {

        auto [width, height] = static_cast<std::pair<float, float>>(MainWindow::getInstance()->get_framebuffer_size());
        width_ = width;
        height_ = height;
        update_proj_to_perspective(
            glm::radians(100.0f),
            width / height,
            {0.1f, 2000.0f});

#ifndef NDEBUG
        std::cout << " camera : " << width << " " << height << "\n";
#endif
        /*
        update_proj_to_perspective(
            glm::radians(45.0f),
            width / height,
            {0.1f, width / 2.0f + 100.0f});
            */
    }
    void MainCamera::update_ortho()
    {

        auto [width, height] = static_cast<std::pair<float, float>>(MainWindow::getInstance()->get_framebuffer_size());
        width_ = width;
        height_ = height;
        view_ = glm::mat4(1.0f);

        update_proj_to_ortho({0.0f, width},
                             {0.0f, height},
                             {-100, 100});
    }

} // namespace vkcpp