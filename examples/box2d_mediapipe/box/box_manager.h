#ifndef BOX_BOX_MANAGER_H
#define BOX_BOX_MANAGER_H

#include "vkcpp.h"
#include <box2d/box2d.h>

namespace box
{
    class BoxManager : public vkcpp::Singleton<BoxManager>
    {
    private:
        std::unique_ptr<b2World> world_;
        std::unique_ptr<vkcpp::Object2D> ground_;
        b2Body *ground_body_;
        float time_step_ = 1.0f / 60.0f;
        int32 velocity_iterations_ = 8;
        int32 position_iterations_ = 3;
        static constexpr float pixels_per_meter_ = 50.0f;
        static constexpr float meter_per_pixels_ = 0.02f;

    public:
        BoxManager();
        ~BoxManager();
        void init_world();
        void init_ground();
        b2Body *create_body(const b2BodyDef *body_def);
        void step();
        static glm::vec2 get_pixels(float meter_x, float meter_y);
        static float get_pixels(float meter);
        static glm::vec2 get_meters(float pixel_x, float pixel_y);
        static float get_meters(float pixel);
    };
}

#endif