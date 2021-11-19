#include "sub_camera.h"

namespace vkcpp
{
    SubCamera::SubCamera(VkExtent3D extent)
    {
        /*
        update_view_to_look_at(
            {0.0f, 0.0f, 10.0f},
            {0.0f, 0.0f, 100.0f},
            {0.0f, 1.0f, 0.0f});
        */

        /*
        update_proj_to_perspective(
            glm::radians(45.0f),
            width / height,
            {0.1f, 100.0f});
        */

        float width = static_cast<float>(extent.width);

        float height = static_cast<float>(extent.height);

        view_ = glm::mat4(1.0f);

        update_proj_to_ortho({0.0f, width},
                             {0.0f, height},
                             {-100, 100});
    }

} // namespace vkcpp