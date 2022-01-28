#ifndef VKCPP_RENDER_OBJECT_OBJECT_2D_H
#define VKCPP_RENDER_OBJECT_OBJECT_2D_H

#include "vulkan_header.h"
#include "shader_attribute.hpp"
#include "render/buffer/uniform_buffers.hpp"

namespace vkcpp
{
    class Camera;

    class GraphicsPipeline;

    class RenderStage;

    class Image2D;

    class CommandPool;

    class CommandBuffers;

    class Model;

    class Device;

    struct TransformComponent
    {
        glm::vec3 translation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 get_mat4();
    };
    /**
     *  TODO: remove renderpass dependency
     *  UBOs count == Swapchain image count
     */
    class Object2D
    {
    protected:
        const Device *device_{nullptr};

        const RenderStage *render_stage_{nullptr};

        const CommandPool *command_pool_{nullptr};

        const char *texture_file_{nullptr};

        std::string vert_shader_file_{"../../../resources/shaders/vs_default.spv"};

        std::string frag_shader_file_{"../../../resources/shaders/fs_default.spv"};

        std::unique_ptr<UniformBuffers<shader::attribute::TransformUBO>> uniform_buffers_{nullptr};

        std::vector<std::shared_ptr<Image2D>> texture_;

        // TODO : renderpass compatiblility and check
        std::shared_ptr<GraphicsPipeline> graphics_pipeline_{nullptr};

        std::shared_ptr<Model> model_{nullptr};

        uint32_t framebuffers_size_{0};

        TransformComponent transform_{};

        int current_texture_{0};

    public:
        Object2D() = default;

        /**
         *  for bin texture (255 255 255 ...)
         */
        Object2D(const Device *device,
                 const RenderStage *render_stage,
                 const CommandPool *command_pool,
                 const VkExtent3D &extent,
                 VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        /**
         *   if texture_file == nullptr, then init is not exe.
         */
        Object2D(const Device *device,
                 const RenderStage *render_stage,
                 const CommandPool *command_pool,
                 const char *texture_file = nullptr,
                 VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        Object2D(const Object2D *);

        Object2D(const Object2D &) = delete;

        Object2D(Object2D &&) = default;

        virtual ~Object2D();

        Object2D &operator=(Object2D &&) = default;

        const GraphicsPipeline *get_graphics_pipeline() const;

        const uint32_t get_framebuffers_size() const;

        const VkExtent3D &get_extent_3d() const;

        const VkImage &get_image() const;

        const VkFormat &get_format() const;

        const TransformComponent &get_transform_component() const;

        const int get_texture_count() const;

        const int get_current_texture_idx() const;

        UniformBuffers<shader::attribute::TransformUBO> &get_mutable_uniform_buffers();

        void init_color(const glm::vec4 &color);

        void prod_color(const glm::vec4 &color);

        virtual void init_transform(const glm::vec3 &translation, const glm::vec3 &scale = glm::vec3(1.0f, 1.0f, 1.0f), const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f));

        void add_transform(const glm::vec3 &translation, const glm::vec3 &scale = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &rotation = glm::vec3(0.0f, 0.0f, 0.0f));

        void move_xy(float new_x, float new_y);

        void rotate_z(float radian);

        virtual void update_with_main_camera(uint32_t uniform_buffer_idx);

        void update_with_sub_camera(uint32_t uniform_buffer_idx, const Camera *sub_camera);

        void update_with_sub_camera(UniformBuffers<shader::attribute::TransformUBO> *external_ubo, uint32_t uniform_buffer_idx, const Camera *sub_camera);

        void init_texture(const VkExtent3D &extent, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        void init_object2d();

        void init_dependency_renderpass(const RenderStage *render_stage);

        void destroy_dependency_renderpass();

        void destroy_object2d();

        void load_model();

        void bind_graphics_pipeline(VkCommandBuffer command_buffer);

        // Draw with internal pipeline and UBO (without bind graphics pipeline)
        virtual void draw_without_bind_graphics(VkCommandBuffer command_buffer, int idx);

        // Draw with external UBO and bind internal graphics pipeline (bind internal graphics pipeline)
        virtual void draw(VkCommandBuffer command_buffer, const UniformBuffers<shader::attribute::TransformUBO> *uniform_buffers, int idx);

        // Draw with internal UBO and bind external pipeline (bind external graphics pipeline)
        virtual void draw(VkCommandBuffer command_buffer, const GraphicsPipeline *graphics_pipeline, int idx);

        // Draw with internal UBO and internal pipeline (bind internal graphics pipeline)
        virtual void draw(VkCommandBuffer command_buffer, int idx);

        void push_texture(const char *texture_file);

        void change_texture(int idx);

        void change_texture(int idx, int ubo_idx);

        void sub_texture(const char *path);

        void sub_texture(VkImage image, VkExtent3D extent);

        /**
         * @return staging image, memory, data, rowpitch
         */
        std::tuple<VkBuffer, VkDeviceMemory, const char *, VkDeviceSize> map_read_image_memory();
        void unmap_buffer_memory(VkBuffer buffer, VkDeviceMemory memory);

        void data_to_file(const char *filename, const char *data, const VkExtent3D &extent, VkFormat image_format, bool supports_blit, VkDeviceSize row_pitch)
        {

            std::ofstream file(filename, std::ios::out | std::ios::binary);

            // ppm header
            file << "P6\n"
                 << extent.width << "\n"
                 << extent.height << "\n"
                 << 255 << "\n";

            // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
            bool colorSwizzle = false;
            // Check if source is BGR
            // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
            if (!supports_blit)
            {
                std::vector<VkFormat> formatsBGR = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
                colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), image_format) != formatsBGR.end());
            }

            // ppm binary pixel data
            for (uint32_t y = 0; y < extent.height; y++)
            {
                unsigned int *row = (unsigned int *)data;
                for (uint32_t x = 0; x < extent.width; x++)
                {
                    if (colorSwizzle)
                    {
                        file.write((char *)row + 2, 1);
                        file.write((char *)row + 1, 1);
                        file.write((char *)row, 1);
                    }
                    else
                    {
                        file.write((char *)row, 3);
                    }
                    row++;
                }
                data += row_pitch;
            }
            file.close();
#ifdef _DEBUG__
            std::cout << "Screenshot saved to disk" << std::endl;
#endif
        }
    }; // class Object
} // namespace vkcpp

#endif // #ifndef VKCPP_RENDER_OBJECT_OBJECT_H