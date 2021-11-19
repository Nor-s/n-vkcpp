#ifndef VKCPP_RENDER_IMAGE_OFFSCREEN_H
#define VKCPP_RENDER_IMAGE_OFFSCREEN_H

#include "image.h"

namespace vkcpp
{
    class Offscreen : public Image
    {
    private:
        VkBuffer staging_buffer_{VK_NULL_HANDLE};

        VkDeviceMemory staging_memory_{VK_NULL_HANDLE};

        VkDeviceSize image_size_{0};

        bool is_mapping{false};

    public:
        explicit Offscreen(const Device *device, const CommandPool *command_pool, const VkExtent3D &extent, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

        virtual ~Offscreen();
        void init_offscreen_image();

        void init_offscreen_view();
        const char *map_image_memory();
        std::unique_ptr<CommandPool> uniq_command_pool_;

        void unmap_memory();
        void screen_to_image(const CommandPool *command_pool, VkImage host_dst_image, const VkExtent3D &src_extent, const VkOffset3D &src_offset, const VkFormat &color_format);
    };

} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_IMAGE_IMAGE_OFFSCREEN_H