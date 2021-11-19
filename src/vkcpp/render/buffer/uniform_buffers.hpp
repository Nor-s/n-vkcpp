#ifndef VKCPP_RENDER_BUFFER_UNIFORM_BUFFERS_HPP
#define VKCPP_RENDER_BUFFER_UNIFORM_BUFFERS_HPP

#include "buffer.hpp"
#include "descriptor_sets.h"
#include "device/device.h"
#include "descriptor_sets.h"
#include "render/image/image.h"

namespace vkcpp
{
    template <typename T>
    class UniformBuffers : public DescriptorSets
    {
    private:
        const Image *image_;

        std::vector<Buffer<T>> handle_;

    public:
        UniformBuffers() = delete;

        UniformBuffers(const Device *device_, const Image *image, uint32_t size);

        virtual ~UniformBuffers();

        const VkImage &get_image() const
        {
            return image_->get_image();
        }
        const VkImageView &get_image_view() const
        {
            return image_->get_image_view();
        }

        const VkSampler &get_sampler() const
        {
            return image_->get_sampler();
        }
        const int get_size() const
        {
            return handle_.size();
        }

        void set_image(const Image *image)
        {
            image_ = image;
            update_descriptor();
        }
        void set_image(const Image *image, int i)
        {
            if (i >= static_cast<int>(handle_.size()))
            {
                return;
            }
            image_ = image;
            update_descriptor(i);
        }

        void init_uniform_buffers();

        void destroy_uniform_buffers();

        void update_uniform_buffer(uint32_t idx, const T &src_data);
        void update_descriptor(int i);
        void update_descriptor();
    };

} // namespace vkcpp

#include "uniform_buffers.tpp"

#endif // #ifndef VKCPP_BUFFER_UNIFORM_BUFFER_H