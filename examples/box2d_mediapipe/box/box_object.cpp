#include "box_object.h"
#include "box_manager.h"

namespace box
{
    BoxObject::BoxObject(
        const vkcpp::Device *device,
        const vkcpp::RenderStage *render_stage,
        const vkcpp::CommandPool *command_pool,
        const VkExtent3D &extent,
        const glm::vec3 &position,
        const b2BodyType &type)
        : vkcpp::Object2D(device, render_stage, command_pool, extent), init_position_(position)
    {
        init_box_object(extent, position, type);
        init_transform(position);
    }
    BoxObject::BoxObject(const BoxObject *box_obj)
        : vkcpp::Object2D(box_obj), init_position_(box_obj->init_position_)
    {
        init_box_object(get_extent_3d(), init_position_, box_obj->body_->GetType());
        init_transform(init_position_);
    }
    BoxObject::~BoxObject()
    {
    }

    void BoxObject::init_box_object(const VkExtent3D &extent,
                                    const glm::vec3 &position,
                                    const b2BodyType &type)
    {
        b2BodyDef bodyDef;
        glm::vec2 meter_pos = BoxManager::get_meters(position.x, position.y);
        glm::vec2 meter_ext = {BoxManager::get_meters(extent.width), BoxManager::get_meters(extent.height)};

        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(meter_pos.x, meter_pos.y);
        bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(this);
        bodyDef.type = type;
        bodyDef.allowSleep = false;
        bodyDef.awake = true;
        body_ = BoxManager::getInstance()->create_body(&bodyDef);
        b2PolygonShape polygon_box;

        polygon_box.SetAsBox(meter_ext.x / 2.0, meter_ext.y / 2.0f);
        if (type == b2_dynamicBody)
        {
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &polygon_box;
            fixtureDef.density = 1.0f;
            fixtureDef.friction = 1.0f;
            body_->CreateFixture(&fixtureDef);
        }
        else
        {
            body_->CreateFixture(&polygon_box, 0.0f);
        }

#ifndef NDEBUG
        std::cout << "box ext: " << meter_ext.x << " " << meter_ext.y << "\n";
        glm::vec2 tmp1 = {BoxManager::get_pixels(meter_ext.x), BoxManager::get_pixels(meter_ext.y)};
        std::cout << "ext: " << tmp1.x << " " << tmp1.y << "\n";
        glm::vec2 tmp2 = BoxManager::get_pixels(meter_pos.x, meter_pos.y);
        std::cout << "box pos: " << meter_pos.x << " " << meter_pos.y << "\n";
        std::cout << " pos: " << tmp2.x << " " << tmp2.y << "\n";
#endif
    }
    void BoxObject::update(uint32_t ubo_idx)
    {
        b2Vec2 pos = body_->GetPosition();
        float angle = body_->GetAngle();
        glm::vec2 pixel_pos = BoxManager::get_pixels(pos.x, pos.y);
        move_xy(pixel_pos.x, pixel_pos.y);
        rotate_z(angle);
        update_with_main_camera(ubo_idx);
    }
    void BoxObject::update_box()
    {
        glm::vec2 box_pos = BoxManager::get_meters(transform_.translation.x, transform_.translation.y);
        body_->SetTransform(b2Vec2{box_pos.x, box_pos.y}, transform_.rotation.z);
    }
}
