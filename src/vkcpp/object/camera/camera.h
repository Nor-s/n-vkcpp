#ifndef VKCPP_RENDER_OBJECT_CAMERA_CAMERA_H
#define VKCPP_RENDER_OBJECT_CAMERA_CAMERA_H

#include "vulkan_header.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkcpp
{
    namespace view
    {
        struct LookAt
        {
            glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 10.0f);
            glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, 100.0f);
            glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::mat4 get_mat4();
        };
        struct Polar
        {
            float radius{};
            float elevation{};
            float azimuth{};
            float twist{};
            // T x Rz X Rx X Ry
            glm::mat4 get_mat4();
        };
        // ToDo implement pilot view
        struct Pilot
        {
            glm::vec3 pos{};
            float roll{};
            float pitch{};
            float yaw{};
            // Rz X Rx X Ry X T
            glm::mat4 get_mat4();
        };
    } // namespace view
    namespace proj
    {
        struct Perspective
        {
            float fovy{};
            float aspect{};
            glm::vec2 z_range{};
            glm::mat4 get_mat4();
        };
        struct Ortho
        {
            glm::vec2 x_range{};
            glm::vec2 y_range{};
            glm::vec2 z_range{};
            glm::mat4 get_mat4();
        };
    } // namespace proj
    class Camera
    {
    public:
    protected:
        glm::mat4 view_ = glm::mat4(1.0f);
        glm::mat4 proj_ = glm::mat4(1.0f);
        view::Pilot pilot_info_{};
        view::LookAt lookat_info_{};
        proj::Ortho ortho_info_{};
        proj::Perspective perspective_info_{};

    public:
        virtual const glm::mat4 &get_view() const;
        virtual const glm::mat4 &get_proj() const;
        virtual void update_view_to_pilot(glm::vec3 pos, float roll, float pitch, float yaw);
        virtual void update_view_to_look_at(glm::vec3 camera_pos, glm::vec3 camera_front, glm::vec3 camera_up);
        virtual void update_proj_to_ortho(glm::vec2 x_range, glm::vec2 y_range, glm::vec2 z_range);
        virtual void update_proj_to_perspective(float fovy, float aspect, glm::vec2 z_range);

        void add_pilot(glm::vec3 pos, float roll = 0.0f, float pitch = 0.0f, float yaw = 0.0f);

        void add_look_at(glm::vec3 camera_pos, glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 0.0f));

        void add_ortho(glm::vec2 x_range, glm::vec2 y_range = {0.0f, 0.0f}, glm::vec2 z_range = {0.0f, 0.0f});

        void add_perspective(float fovy, float aspect = 0.0f, glm::vec2 z_range = {0, 0});

        void forward_look_at(float forward = 0.1f);
    };

} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_OBJECT_OBJECT_H