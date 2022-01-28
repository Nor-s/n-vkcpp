#ifndef VKCPP_RENDER_OBJECT_CAMERA_MAIN_CAMERA_H
#define VKCPP_RENDER_OBJECT_CAMERA_MAIN_CAMERA_H

#include "camera.h"
#include "pattern/singleton.hpp"

namespace vkcpp
{
    class MainCamera : public Camera, public Singleton<MainCamera>
    {
    public:
        MainCamera();

        void update();
        void update_ortho();

    private:
        float width_;
        float height_;

    public:
        float get_width() const { return width_; }
        float get_height() const { return height_; }
    };
}

#endif