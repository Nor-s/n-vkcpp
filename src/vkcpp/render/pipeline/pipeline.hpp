#ifndef VKCPP_SWAPCHAIN_PIPELINE_H
#define VKCPP_SWAPCHAIN_PIPELINE_H

#include "vulkan_header.h"

namespace vkcpp
{
    class CommandBuffer;

    class Pipeline
    {
    public:
        struct CreateInfo
        {
            VkPipelineVertexInputStateCreateInfo vertex_input_state{};

            VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};

            VkPipelineViewportStateCreateInfo viewport_state{};

            VkPipelineRasterizationStateCreateInfo rasterizer_state{};

            VkPipelineMultisampleStateCreateInfo multisample_state{};

            VkPipelineColorBlendStateCreateInfo color_blend_state{};

            VkPipelineDynamicStateCreateInfo dynamic_state{};

            std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
        };

        Pipeline() = default;

        virtual ~Pipeline() = default;

        void bind_pipeline(const VkCommandBuffer &command_buffer) const
        {
            vkCmdBindPipeline(command_buffer, get_pipeline_bind_point(), get_pipeline());
        }
        virtual const VkPipelineLayout &get_pipeline_layout() const = 0;

        virtual const VkPipeline &get_pipeline() const = 0;

        virtual const VkPipelineBindPoint &get_pipeline_bind_point() const = 0;

    }; // class GraphicsPipeline
} // namespace vkcpp

#endif