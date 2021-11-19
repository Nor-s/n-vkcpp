#include "model.h"

#include "device/device.h"
#include "render/swapchain/swapchain.h"
#include "render/command/command_pool.h"
#include "render/command/command_buffers.h"
#include "render/pipeline/graphics_pipeline.h"
#include "render/image/image.h"
#include "render/render_stage.h"

namespace vkcpp
{
    Model::Model(const Device *device,
                 const CommandPool *command_pool,
                 std::vector<shader::attribute::Vertex> &vertices)
        : device_(device), command_pool_(command_pool), vertices_(std::move(vertices))
    {
        init_model();
    }

    Model::~Model()
    {
        destroy_model();
    }

    void Model::init_model()
    {
        index_buffer_ = std::make_unique<Buffer<uint16_t>>(
            device_,
            command_pool_,
            &indices_,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            true);

        vertex_buffer_ = std::make_unique<Buffer<shader::attribute::Vertex>>(
            device_,
            command_pool_,
            &vertices_,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            true);
    }

    void Model::destroy_model()
    {
        vertex_buffer_.reset();
        index_buffer_.reset();
    }

    void Model::bind(VkCommandBuffer command_buffer)
    {
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer_->get_buffer(), offsets);

        vkCmdBindIndexBuffer(command_buffer, *index_buffer_, 0, VK_INDEX_TYPE_UINT16);
    }

    void Model::draw(VkCommandBuffer command_buffer)
    {
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
    }
}
