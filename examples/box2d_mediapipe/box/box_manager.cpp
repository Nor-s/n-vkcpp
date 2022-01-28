#include "box_manager.h"

namespace box
{
    BoxManager::BoxManager()
    {
        init_world();
        init_ground();
    }
    BoxManager::~BoxManager()
    {
        world_.reset();
    }
    void BoxManager::init_world()
    {
        b2Vec2 gravity(0.0f, -10.0f);
        world_ = std::make_unique<b2World>(gravity);
    }
    void BoxManager::init_ground()
    {
        b2BodyDef ground_body_def;
        float height = vkcpp::MainCamera::getInstance()->get_height();
        glm::vec2 pos = get_meters(0.0f, height * 2 - 100.0f);
        glm::vec2 ext = {get_meters(1000.0f), get_meters(height)};

        ground_body_def.position.Set(pos.x, pos.y);
        ground_body_ = create_body(&ground_body_def);

        b2PolygonShape ground_box;

        ground_box.SetAsBox(ext.x, ext.y);
        ground_body_->CreateFixture(&ground_box, 0.0f);
    }
    b2Body *BoxManager::create_body(const b2BodyDef *body_def)
    {
        return world_->CreateBody(body_def);
    }
    void BoxManager::step()
    {
        world_->Step(time_step_, velocity_iterations_, position_iterations_);
    }
    glm::vec2 BoxManager::get_pixels(float meter_x, float meter_y)
    {
        float height = vkcpp::MainCamera::getInstance()->get_height();
        return glm::vec2{pixels_per_meter_ * meter_x, height - pixels_per_meter_ * meter_y};
    }
    float BoxManager::get_pixels(float meter)
    {
        return meter * pixels_per_meter_;
    }
    glm::vec2 BoxManager::get_meters(float pixel_x, float pixel_y)
    {
        float height = vkcpp::MainCamera::getInstance()->get_height();
        return glm::vec2{meter_per_pixels_ * pixel_x, meter_per_pixels_ * (height - pixel_y)};
    }
    float BoxManager::get_meters(float pixel)
    {
        return pixel * meter_per_pixels_;
    }
}
