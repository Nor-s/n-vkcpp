#include "class/brush.h"

#include "utility/utility.h"

namespace painting
{
    Brushes::Brushes(const vkcpp::Device *device,
                     const vkcpp::RenderStage *render_stage,
                     const vkcpp::CommandPool *command_pool,
                     int brush_count)
    {
        brushes_.push_back(std::make_unique<vkcpp::Object2D>(
            device,
            render_stage,
            command_pool,
            tex_[rand() % TEX_SIZE_]));
        for (int i = 1; i < brush_count; i++)
        {
            brushes_.push_back(std::make_unique<vkcpp::Object2D>(
                brushes_[0].get()));
        }
    }

    void Brushes::draw_all(VkCommandBuffer command_buffer, int ubo_idx)
    {
        int size = brushes_.size();
        brushes_[0]->bind_graphics_pipeline(command_buffer);
        for (int i = 0; i < size; i++)
        {
            brushes_[i]->draw_without_bind_graphics(command_buffer, ubo_idx);
        }
    }

    void Brushes::update(const BrushAttributeComponent &attribute, const vkcpp::Camera *camera, int idx, int ubo_idx)
    {
        brushes_[idx]
            ->init_transform(
                attribute.translation,
                attribute.scale,
                glm::vec3(0.0f, 0.0f, attribute.rotation_z));
        brushes_[idx]->init_color(attribute.color);
        //  brushes_[idx]->change_texture(attribute.object_idx, ubo_idx);
        brushes_[idx]->update_with_sub_camera(ubo_idx, camera);
    }
}

namespace painting
{
    BrushAttributes::BrushAttributes(const glm::vec2 &offset,
                                     const glm::vec2 &extent,
                                     const glm::vec2 &scale_range,
                                     int size,
                                     const Probablity &probablity)
        : offset_(offset),
          extent_(extent),
          probablity_(probablity)
    {
        for (int i = 0; i < size; i++)
        {
            attributes_.push_back(BrushAttributeComponent(scale_range));
            set_rand_rotation(i);
            set_rand_scale(i);
            set_rand_translation(i);
            set_rand_color(i);
            set_rand_obj_idx(i);
        }
    }
    BrushAttributes::BrushAttributes(const BrushAttributes &a, const BrushAttributes &b)
    {
        int size = a.attributes_.size();
        for (int i = 0; i < size; i++)
        {
            if (rand() % 2 == 0)
            {
                attributes_.push_back(a.attributes_[i]);
            }
            else
            {
                attributes_.push_back(b.attributes_[i]);
            }
        }
        offset_ = a.offset_;
        extent_ = a.extent_;
        probablity_ = a.probablity_;
        fitness_ = a.fitness_;
    }

    BrushAttributes::~BrushAttributes()
    {
    }

    std::unique_ptr<BrushAttributes> BrushAttributes::cross_over(const BrushAttributes &b)
    {
        return std::make_unique<BrushAttributes>(*this, b);
    }
    void BrushAttributes::mutate(int idx)
    {
        if (rand() % 4 == 0)
        {
            if (vkcpp::getProbablity() < probablity_.scale)
            {
                set_rand_scale(idx);
            }
            if (vkcpp::getProbablity() < probablity_.trans)
            {
                set_rand_translation(idx);
            }
            if (vkcpp::getProbablity() < probablity_.rotate)
            {
                set_rand_rotation(idx);
            }
            if (vkcpp::getProbablity() < probablity_.color)
            {
                set_rand_color(idx);
            }
        }
        else
        {
            set_rand_color(idx, true);
            set_rand_rotation(idx);
            set_rand_scale(idx, true);
            set_rand_translation(idx, true);
        }
    }
    void BrushAttributes::set_rand_obj_idx(int idx)
    {
        BrushAttributeComponent &attribute = attributes_[idx];
        attribute.object_idx = rand() % Brushes::TEX_SIZE_;
    }
    void BrushAttributes::set_rand_scale(int idx, bool is_relative)
    {
        BrushAttributeComponent &attribute = attributes_[idx];
        if (is_relative)
        {
            attribute.scale.x = std::clamp(vkcpp::getRandFloat(attribute.scale.x - 0.001f, attribute.scale.x + 0.001f), attribute.scale_range_.x, attribute.scale_range_.y);
            attribute.scale.y = std::clamp(vkcpp::getRandFloat(attribute.scale.y - 0.001f, attribute.scale.y + 0.001f), attribute.scale_range_.x, attribute.scale_range_.y);
        }
        else
        {
            attribute.scale.x = vkcpp::getRandFloat(attribute.scale_range_.x, attribute.scale_range_.y);
            attribute.scale.y = vkcpp::getRandFloat(attribute.scale_range_.x, attribute.scale_range_.y);
        }
    }
    void BrushAttributes::set_rand_translation(int idx, bool is_relative)
    {
        BrushAttributeComponent &attribute = attributes_[idx];
        if (is_relative)
        {
            attribute.translation.x = std::clamp(vkcpp::getRandFloat(attribute.translation.x - 15.0f, attribute.translation.x + 15.0f), offset_.x, offset_.x + extent_.x);
            attribute.translation.y = std::clamp(vkcpp::getRandFloat(attribute.translation.y - 15.0f, attribute.translation.y + 15.0f), offset_.y, offset_.y + extent_.y);
            attribute.translation.z = -std::clamp(vkcpp::getRandFloat(attribute.translation.z - 5.0f, attribute.translation.z + 5.0f), 1.0f, 50.0f);
        }
        else
        {
            attribute.translation.x = vkcpp::getRandFloat(offset_.x, offset_.x + extent_.x);
            attribute.translation.y = vkcpp::getRandFloat(offset_.y, offset_.y + extent_.y);
            attribute.translation.z = vkcpp::getRandFloat(1.0f, 50.0f);
        }
    }
    void BrushAttributes::set_rand_rotation(int idx, bool is_relative)
    {
        BrushAttributeComponent &attribute = attributes_[idx];
        if (is_relative)
        {
            attribute.rotation_z = std::clamp(vkcpp::getRandFloat(attribute.rotation_z - 0.3f, attribute.rotation_z + 0.3f), 0.0f, 6.3f);
        }
        else
        {
            attribute.rotation_z = vkcpp::getRandFloat(0.0f, 6.3f);
        }
    }
    void BrushAttributes::set_rand_color(int idx, bool is_relative)
    {
        BrushAttributeComponent &attribute = attributes_[idx];
        if (is_relative)
        {
            attribute.color.r = std::clamp(vkcpp::getRandFloat(attribute.color.r - 0.02f, attribute.color.r + 0.02f), 0.0f, 1.0f);
            attribute.color.g = std::clamp(vkcpp::getRandFloat(attribute.color.g - 0.02f, attribute.color.g + 0.02f), 0.0f, 1.0f);
            attribute.color.b = std::clamp(vkcpp::getRandFloat(attribute.color.b - 0.02f, attribute.color.b + 0.02f), 0.0f, 1.0f);
            attribute.color.a = std::clamp(vkcpp::getRandFloat(attribute.color.a, attribute.color.a + 0.001f), 0.0f, 1.0f);
        }
        else
        {
            attribute.color.r = vkcpp::getRandFloat(0.0f, 1.0f);
            attribute.color.g = vkcpp::getRandFloat(0.0f, 1.0f);
            attribute.color.b = vkcpp::getRandFloat(0.0f, 1.0f);
            attribute.color.a = vkcpp::getRandFloat(0.0f, 1.0f);
        }
    }
}