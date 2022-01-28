#include "hand_object.h"

namespace hand
{
    const std::vector<std::pair<int, int>> HandObject::landmark_pair_ = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {0, 5}, {5, 6}, {6, 7}, {7, 8}, {5, 9}, {9, 10}, {10, 11}, {11, 12}, {9, 13}, {13, 14}, {14, 15}, {15, 16}, {13, 17}, {17, 18}, {18, 19}, {19, 20}, {0, 17}};
    const HandInput HandObject::hand_input_;
    const float HandObject::hand_scale_ = 0.7f;

    HandObject::HandObject(const vkcpp::Device *device,
                           const vkcpp::RenderStage *render_stage,
                           const vkcpp::CommandPool *command_pool)
    {
        hand_seg_.push_back(std::make_unique<box::BoxObject>(
            device,
            render_stage,
            command_pool,
            VkExtent3D{static_cast<uint32_t>(20 * hand_scale_), 1, 1},
            glm::vec3{0.0f, 0.0f, -90.0f},
            b2_staticBody));
        for (int i = 1; i < 21; i++)
        {
            hand_seg_.push_back(std::make_unique<box::BoxObject>(
                hand_seg_[0].get()));
        }
        width_ = render_stage->get_render_area().extent.width;
        height_ = render_stage->get_render_area().extent.height;
    }
    HandObject::~HandObject()
    {
        hand_seg_.clear();
    }
    void HandObject::draw_all(VkCommandBuffer command_buffer, int ubo_idx)
    {
        hand_seg_[0]->bind_graphics_pipeline(command_buffer);
        for (int i = 0; i < 21; i++)
        {
            hand_seg_[i]->draw_without_bind_graphics(command_buffer, ubo_idx);
        }
    }
    glm::vec3 HandObject::cal_scale(int idx, const std::pair<glm::vec3, glm::vec3> &p)
    {
        float x = (p.first.x - p.second.x) * width_;
        float y = (p.first.y - p.second.y) * height_;
        float len = sqrt(x * x + y * y);
        return {1.0f, len, 1.0f};
    }
    glm::vec3 HandObject::cal_trans(int idx, const std::pair<glm::vec3, glm::vec3> &p)
    {
        float x = (p.first.x + p.second.x) * width_ / 2.0f;
        float y = (p.first.y + p.second.y) * height_ / 2.0f;
        return {x, y, -90.0f};
    }
    glm::vec3 HandObject::cal_rotate(int idx, const std::pair<glm::vec3, glm::vec3> &p)
    {
        float x = (p.first.x - p.second.x) * width_;
        float y = (p.first.y - p.second.y) * height_;
        float angle = atan2f(y, x) + 90.0f * glm::pi<float>() / 180.0f;
        return {0.0f, 0.0f, angle};
    }
    glm::vec3 HandObject::resize(const glm::vec3 &center, const glm::vec3 &p, float hand_scale)
    {
        return glm::vec3{center.x + (p.x - center.x) * hand_scale, center.y + (p.y - center.y) * hand_scale, p.z};
    }
    void HandObject::update_all(int ubo_idx)
    {
        glm::vec3 center = hand_input_.getLandmarks(0);
        for (int i = 0; i < 21; i++)
        {
            std::pair<int, int> land_idx = landmark_pair_[i];
            glm::vec3 first = resize(center, hand_input_.getLandmarks(land_idx.first), hand_scale_);
            glm::vec3 second = resize(center, hand_input_.getLandmarks(land_idx.second), hand_scale_);
            hand_seg_[i]->init_transform(cal_trans(i, {first, second}), cal_scale(i, {first, second}), cal_rotate(i, {first, second}));
            hand_seg_[i]->update_with_main_camera(ubo_idx);
            hand_seg_[i]->update_box();
        }
    }

}