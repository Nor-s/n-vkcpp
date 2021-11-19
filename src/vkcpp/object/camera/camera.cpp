#include "camera.h"

namespace vkcpp
{
    inline glm::mat4 view::LookAt::get_mat4()
    {
        return glm::lookAt(camera_pos,
                           camera_pos - camera_front,
                           camera_up);
    }

    inline glm::mat4 view::Polar::get_mat4()
    {
        glm::mat4 ret = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
        ret = glm::rotate(ret, -twist, glm::vec3(0.0f, 0.0f, 1.0f));
        ret = glm::rotate(glm::mat4(1.0f), -elevation, glm::vec3(1.0f, 0.0f, 0.0f));
        ret = glm::rotate(glm::mat4(1.0f), -azimuth, glm::vec3(0.0f, 1.0f, 0.0f));

        return ret;
    }

    inline glm::mat4 view::Pilot::get_mat4()
    {
        glm::mat4 ret = glm::rotate(glm::mat4(1.0f), -roll, glm::vec3(0.0f, 0.0f, 1.0f));
        ret = glm::rotate(ret, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        ret = glm::rotate(ret, -yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        ret = glm::translate(ret, glm::vec3(-pos.x, -pos.y, -pos.z));

        return ret;
    }

    inline glm::mat4 proj::Perspective::get_mat4()
    {
        return glm::perspective(fovy, aspect, z_range.x, z_range.y);
    }
    inline glm::mat4 proj::Ortho::get_mat4()
    {
        return glm::ortho(x_range.x, x_range.y, y_range.x, y_range.y, z_range.x, z_range.y);
    }
    const glm::mat4 &Camera::get_view() const
    {
        return view_;
    }
    const glm::mat4 &Camera::get_proj() const
    {
        return proj_;
    }

    void Camera::update_view_to_pilot(glm::vec3 pos, float roll, float pitch, float yaw)
    {
        pilot_info_.pos = pos;
        pilot_info_.roll = roll;
        pilot_info_.pitch = pitch;
        pilot_info_.yaw = yaw;
        view_ = pilot_info_.get_mat4();
    }
    void Camera::update_view_to_look_at(glm::vec3 camera_pos, glm::vec3 camera_front, glm::vec3 camera_up)
    {
        lookat_info_.camera_pos = camera_pos;
        lookat_info_.camera_front = camera_front;
        lookat_info_.camera_up = camera_up;
        view_ = lookat_info_.get_mat4();
    }
    void Camera::forward_look_at(float forward)
    {
        lookat_info_.camera_pos += glm::normalize(lookat_info_.camera_front) * forward;
        view_ = lookat_info_.get_mat4();
    }
    void Camera::update_proj_to_ortho(glm::vec2 x_range, glm::vec2 y_range, glm::vec2 z_range)
    {
        ortho_info_.x_range = x_range;
        ortho_info_.y_range = y_range;
        ortho_info_.z_range = z_range;
        proj_ = ortho_info_.get_mat4();
    }
    void Camera::update_proj_to_perspective(float fovy, float aspect, glm::vec2 z_range)
    {
        perspective_info_.fovy = fovy;
        perspective_info_.aspect = aspect;
        perspective_info_.z_range = z_range;
        proj_ = perspective_info_.get_mat4();
    }
    // Todo modulo 360
    void Camera::add_pilot(glm::vec3 pos, float roll, float pitch, float yaw)
    {
        pilot_info_.pos += pos;
        pilot_info_.roll += roll;
        pilot_info_.pitch += pitch;
        pilot_info_.yaw += yaw;
        view_ = pilot_info_.get_mat4();
    }

    void Camera::add_look_at(glm::vec3 camera_pos, glm::vec3 camera_front, glm::vec3 camera_up)
    {
        lookat_info_.camera_pos += camera_pos;
        lookat_info_.camera_front += camera_front;
        lookat_info_.camera_up += camera_up;
        view_ = lookat_info_.get_mat4();
    }
    void Camera::add_ortho(glm::vec2 x_range, glm::vec2 y_range, glm::vec2 z_range)
    {
        ortho_info_.x_range += x_range;
        ortho_info_.y_range += y_range;
        ortho_info_.z_range += z_range;

        proj_ = ortho_info_.get_mat4();
    }

    void Camera::add_perspective(float fovy, float aspect, glm::vec2 z_range)
    {
        perspective_info_.fovy += fovy;
        perspective_info_.aspect += aspect;
        perspective_info_.z_range.x += z_range.x;
        perspective_info_.z_range.x += z_range.y;

        proj_ = perspective_info_.get_mat4();
    }
}