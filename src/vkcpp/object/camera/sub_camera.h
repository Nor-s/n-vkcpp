#ifndef VKCPP_RENDER_OBJECT_CAMERA_SUB_CAMERA_H
#define VKCPP_RENDER_OBJECT_CAMERA_SUB_CAMERA_H

#include "camera.h"

namespace vkcpp
{
    class SubCamera : public Camera
    {
    public:
        SubCamera(VkExtent3D extent);
    };
}

#endif