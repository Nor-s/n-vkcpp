#ifndef VKCPP_RENDER_IMAGE_IMAGE_DEPTH_H
#define VKCPP_RENDER_IMAGE_IMAGE_DEPTH_H

#include "image.h"

namespace vkcpp
{
    class ImageDepth : public Image
    {
    private:
    public:
        explicit ImageDepth(const Device *device, const CommandPool *command_pool, VkExtent3D extent, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
        virtual ~ImageDepth() = default;
        void init_depth_image();
        void init_depth_view();
        void init_depth_sampler();
    };

} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_IMAGE_IMAGE_DEPTH_H