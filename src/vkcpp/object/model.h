#ifndef VKCPP_RENDER_OBJECT_MODEL_H
#define VKCPP_RENDER_OBJECT_MODEL_H

#include "vulkan_header.h"

#include "shader_attribute.hpp"
#include "render/buffer/uniform_buffers.hpp"

namespace vkcpp
{
    class Model
    {
    private:
        const Device *device_{nullptr};

        const CommandPool *command_pool_{nullptr};

        //interleaving vertex attributes
        std::vector<shader::attribute::Vertex> vertices_;

        std::vector<uint16_t> indices_ = {0, 1, 2, 2, 3, 0};

        std::unique_ptr<Buffer<shader::attribute::Vertex>> vertex_buffer_{nullptr};

        std::unique_ptr<Buffer<uint16_t>> index_buffer_{nullptr};

    public:
        Model(const Device *device,
              const CommandPool *command_pool,
              std::vector<shader::attribute::Vertex> &vertices);

        Model(const Model &) = delete;

        Model(Model &&) = default;

        virtual ~Model();

        Model &operator=(Model &&) = default;

        void init_model();

        void destroy_model();

        void bind(VkCommandBuffer command_buffer);

        void draw(VkCommandBuffer command_buffer);

    }; // class Model
} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_Model_Model_H