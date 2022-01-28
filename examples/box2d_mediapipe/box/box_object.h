#ifndef BOX_BOX_OBJECT_H
#define BOX_BOX_OBJECT_H

#include "vkcpp.h"
#include <box2d/box2d.h>

namespace box
{
    class BoxObject : public vkcpp::Object2D
    {
    private:
        b2Body *body_ = nullptr;
        glm::vec3 init_position_;

    public:
        BoxObject() = delete;
        BoxObject(const BoxObject *);
        BoxObject(const BoxObject &) = delete;
        BoxObject(
            const vkcpp::Device *device,
            const vkcpp::RenderStage *render_stage,
            const vkcpp::CommandPool *command_pool,
            const VkExtent3D &extent,
            const glm::vec3 &position,
            const b2BodyType &type = b2_dynamicBody);

        virtual ~BoxObject();
        void init_box_object(const VkExtent3D &extent,
                             const glm::vec3 &position,
                             const b2BodyType &type = b2_dynamicBody);
        virtual void update(uint32_t ubo_idx);
        virtual void update_box();
    };
}

#endif