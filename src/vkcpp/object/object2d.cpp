#include "object2d.h"

#include "model.h"
#include "device/device.h"
#include "object/camera/main_camera.h"
#include "render/image/image.h"
#include "render/image/image2d.h"
#include "render/render_stage.h"
#include "render/swapchain/swapchain.h"
#include "render/command/command_pool.h"
#include "render/command/command_buffers.h"
#include "render/pipeline/graphics_pipeline.h"
#include "object/camera/camera.h"

namespace vkcpp
{
    glm::mat4 TransformComponent::get_mat4()
    {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        glm::mat4 ret(1.0f);
        ret = glm::translate(ret, translation);
        ret = glm::rotate(ret, rotation.z, {0.0f, 0.0f, 1.0f});
        ret = glm::scale(ret, scale);
        return

            {

                {
                    scale.x * (c1 * c3 + s1 * s2 * s3),
                    scale.x * (c2 * s3),
                    scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    scale.y * (c3 * s1 * s2 - c1 * s3),
                    scale.y * (c2 * c3),
                    scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    scale.z * (c2 * s1),
                    scale.z * (-s2),
                    scale.z * (c1 * c2),
                    0.0f,
                },
                {translation.x, translation.y, translation.z, 1.0f}};
    }

    Object2D::Object2D(const Device *device,
                       const RenderStage *render_stage,
                       const CommandPool *command_pool,
                       const VkExtent3D &extent,
                       VkFormat format)
        : device_(device), render_stage_(render_stage), command_pool_(command_pool), texture_file_(nullptr), current_texture_(0)
    {
        init_texture(extent, format);
        init_object2d();
    }
    Object2D::Object2D(const Object2D *a)
        : device_(a->device_),
          render_stage_(a->render_stage_),
          command_pool_(a->command_pool_),
          texture_file_(a->texture_file_),
          framebuffers_size_(a->framebuffers_size_),
          transform_(a->transform_),
          current_texture_(a->current_texture_)
    {
        model_ = a->model_;
        graphics_pipeline_ = a->graphics_pipeline_;
        int size = static_cast<int>(a->texture_.size());
        for (int i = 0; i < size; i++)
        {
            texture_.push_back(a->texture_[i]);
        }
        uniform_buffers_ = std::make_unique<UniformBuffers<shader::attribute::TransformUBO>>(
            device_,
            texture_[current_texture_].get(),
            framebuffers_size_);
    }
    Object2D::Object2D(const Device *device,
                       const RenderStage *render_stage,
                       const CommandPool *command_pool,
                       const char *texture_file,
                       VkFormat format)
        : device_(device), render_stage_(render_stage), command_pool_(command_pool), texture_file_(texture_file), current_texture_(0)
    {
        if (texture_file != nullptr)
        {
            init_texture({}, format);
            init_object2d();
        }
#ifdef _DEBUG__
        std::cout << "  render stage object framebufer_size_ : " << framebuffers_size_ << "\n";
#endif
    }

    Object2D::~Object2D()
    {
        destroy_dependency_renderpass();
        destroy_object2d();
    }
    const VkExtent3D &Object2D::get_extent_3d() const
    {
        return texture_[current_texture_]->get_extent();
    }
    const GraphicsPipeline *Object2D::get_graphics_pipeline() const
    {
        return graphics_pipeline_.get();
    }
    const uint32_t Object2D::get_framebuffers_size() const
    {
        return framebuffers_size_;
    }
    const VkImage &Object2D::get_image() const
    {
        return texture_[current_texture_]->get_image();
    }
    const VkFormat &Object2D::get_format() const
    {
        return texture_[current_texture_]->get_format();
    }
    UniformBuffers<shader::attribute::TransformUBO> &Object2D::get_mutable_uniform_buffers()
    {
        return *uniform_buffers_;
    }
    const TransformComponent &Object2D::get_transform_component() const
    {
        return transform_;
    }
    const int Object2D::get_texture_count() const
    {
        return texture_.size();
    }
    const int Object2D::get_current_texture_idx() const
    {
        return current_texture_;
    }

    void Object2D::init_color(const glm::vec4 &color)
    {
        transform_.color = color;
    }
    void Object2D::prod_color(const glm::vec4 &color)
    {
        transform_.color *= color;
    }

    void Object2D::init_transform(const glm::vec3 &translation, const glm::vec3 &scale, const glm::vec3 &rotation)
    {
        transform_.translation = translation;
        transform_.scale = scale;
        transform_.rotation = rotation;
    }
    // TODO handle overflow
    void Object2D::add_transform(const glm::vec3 &translation, const glm::vec3 &scale, const glm::vec3 &rotation)
    {
        transform_.translation += translation;
        transform_.scale += scale;
        transform_.rotation += rotation;
    }

    void Object2D::update_with_main_camera(uint32_t uniform_buffer_idx)
    {
        update_with_sub_camera(uniform_buffer_idx, MainCamera::getInstance());
    }
    void Object2D::update_with_sub_camera(uint32_t uniform_buffer_idx, const Camera *sub_camera)
    {
        vkcpp::shader::attribute::TransformUBO ubo{};
        ubo.model = transform_.get_mat4();
        ubo.color = transform_.color;
        ubo.view = sub_camera->get_view();
        ubo.proj = sub_camera->get_proj();

        // ubo.proj[1][1] *= -1;
        get_mutable_uniform_buffers().update_uniform_buffer(uniform_buffer_idx, ubo);
    }

    void Object2D::update_with_sub_camera(UniformBuffers<shader::attribute::TransformUBO> *external_ubo, uint32_t uniform_buffer_idx, const Camera *sub_camera)
    {
        vkcpp::shader::attribute::TransformUBO ubo{};
        ubo.model = transform_.get_mat4();
        ubo.color = transform_.color;
        ubo.view = sub_camera->get_view();
        ubo.proj = sub_camera->get_proj();

        // ubo.proj[1][1] *= -1;
        external_ubo->update_uniform_buffer(uniform_buffer_idx, ubo);
    }

    void Object2D::init_texture(const VkExtent3D &extent, VkFormat format)
    {
        if (texture_file_ != nullptr)
        {
            texture_.push_back(std::make_unique<Image2D>(
                device_,
                command_pool_,
                texture_file_,
                format));
            current_texture_ = texture_.size() - 1;
        }
        else
        {
            texture_.push_back(std::make_unique<Image2D>(
                device_,
                command_pool_,
                extent,
                format));
            current_texture_ = texture_.size() - 1;
        }
    }

    void Object2D::init_object2d()
    {
        load_model();
        init_dependency_renderpass(render_stage_);
    }

    void Object2D::load_model()
    {
        auto [width, height] = texture_[current_texture_]->get_size();

#ifdef _DEBUG__
        //  std::cout << "       Object load_ model : " << screen_width << " ," << screen_height << "\n";
#endif

        float w = static_cast<float>(width) / 2.0f, h = static_cast<float>(height) / 2.0f;

        // interleaving vertex attributes
        std::vector<shader::attribute::Vertex> vertices = {
            {{-w, -h, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
            {{w, -h, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{w, h, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-w, h, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

        model_ = std::make_shared<Model>(device_, command_pool_, vertices);
    }

    void Object2D::init_dependency_renderpass(const RenderStage *render_stage)
    {
        render_stage_ = render_stage;
        framebuffers_size_ = render_stage_->get_framebuffers().get_framebuffers_size();

        if (uniform_buffers_ != nullptr)
        {
            uniform_buffers_.reset();
        }

        uniform_buffers_ = std::make_unique<UniformBuffers<shader::attribute::TransformUBO>>(
            device_,
            texture_[current_texture_].get(),
            framebuffers_size_);

        if (graphics_pipeline_ != nullptr)
        {
            graphics_pipeline_.reset();
        }

        graphics_pipeline_ = std::make_unique<GraphicsPipeline>(
            device_,
            render_stage_,
            uniform_buffers_.get(),
            vert_shader_file_,
            frag_shader_file_,
            0);
    }

    void Object2D::destroy_dependency_renderpass()
    {
        graphics_pipeline_ = nullptr;
        uniform_buffers_.reset();
    }

    void Object2D::destroy_object2d()
    {
        model_ = nullptr;
        int size = texture_.size();
        for (int i = 0; i < size; i++)
        {
            texture_[i].reset();
        }
    }

    void Object2D::bind_graphics_pipeline(VkCommandBuffer command_buffer)
    {
        graphics_pipeline_->bind_pipeline(command_buffer);
    }

    void Object2D::draw_without_bind_graphics(VkCommandBuffer command_buffer, int ubo_idx)
    {
        model_->bind(command_buffer);

        vkCmdBindDescriptorSets(
            command_buffer,
            graphics_pipeline_->get_pipeline_bind_point(),
            graphics_pipeline_->get_pipeline_layout(),
            0,
            1,
            &uniform_buffers_->get_sets()[ubo_idx],
            0,
            nullptr);

        model_->draw(command_buffer);
    }

    void Object2D::draw(VkCommandBuffer command_buffer, const GraphicsPipeline *graphics_pipeline, int idx)
    {
        graphics_pipeline->bind_pipeline(command_buffer);

        model_->bind(command_buffer);

        vkCmdBindDescriptorSets(
            command_buffer,
            graphics_pipeline->get_pipeline_bind_point(),
            graphics_pipeline->get_pipeline_layout(),
            0,
            1,
            &uniform_buffers_->get_sets()[idx],
            0,
            nullptr);

        model_->draw(command_buffer);
    }

    void Object2D::draw(VkCommandBuffer command_buffer, const UniformBuffers<shader::attribute::TransformUBO> *uniform_buffers, int idx)
    {
        graphics_pipeline_->bind_pipeline(command_buffer);

        model_->bind(command_buffer);

        vkCmdBindDescriptorSets(
            command_buffer,
            graphics_pipeline_->get_pipeline_bind_point(),
            graphics_pipeline_->get_pipeline_layout(),
            0,
            1,
            &uniform_buffers->get_sets()[idx],
            0,
            nullptr);

        model_->draw(command_buffer);
    }

    void Object2D::draw(VkCommandBuffer command_buffer, int idx)
    {
        draw(command_buffer, graphics_pipeline_.get(), idx);
    }

    void Object2D::push_texture(const char *texture_file)
    {
        texture_.push_back(std::make_unique<Image2D>(
            device_,
            command_pool_,
            texture_file));
    }

    void Object2D::change_texture(int idx)
    {
        if (texture_.size() <= static_cast<size_t>(idx))
        {
#ifdef _DEBUG__
            std::cout << "failed to change_texture! out of bounds!\n";
#endif
            return;
        }
        current_texture_ = idx;
        uniform_buffers_->set_image(texture_[idx].get());
    }
    void Object2D::change_texture(int idx, int ubo_idx)
    {
        if (texture_.size() <= static_cast<size_t>(idx))
        {
#ifdef _DEBUG__
            std::cout << "failed to change_texture! out of bounds!\n";
#endif
            return;
        }
        current_texture_ = idx;
        uniform_buffers_->set_image(texture_[idx].get(), ubo_idx);
    }
    void Object2D::sub_texture(const char *path)
    {
        texture_[current_texture_]->sub_texture_image(path);
    }
    // TODO: fix hard coding "format = RGBA SRGB"
    void Object2D::sub_texture(VkImage image, VkExtent3D extent)
    {
        texture_[current_texture_]->sub_image(image, extent, VK_FORMAT_R8G8B8A8_SRGB);
    }

    // TODO : fix hard coding "supportsBlit = false"
    std::tuple<VkBuffer, VkDeviceMemory, const char *, VkDeviceSize> Object2D::map_read_image_memory()
    {
        const vkcpp::Device *device = device_;
        const vkcpp::CommandPool *command_pool = command_pool_;
        //VkFormat format = get_format();
        // bool supportsBlit = true; //= device_->check_support_blit(object->get_);
        VkImage src_image = get_image();
        VkExtent3D extent = get_extent_3d();
        VkDeviceSize size = extent.width * 4 * extent.height;
        if (get_format() == VK_FORMAT_R8G8B8_SRGB)
        {
            size = (extent.width + extent.width % 4) * extent.height * 3;
        }
        else
        {
            size = extent.width * extent.height * 4;
        }
        // Source for the copy is the last rendered swapchain image
        // VkImage srcImage = swapChain.images[currentBuffer];

        // Create the linear tiled destination image to copy to and to read the memory from
        // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
        VkBuffer dst_buffer;
        // Create memory to back up the image
        VkDeviceMemory dst_memory{nullptr};
        vkcpp::create::buffer(
            device_,
            size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            dst_buffer,
            dst_memory);

        // Do the actual blit from the swapchain image to our host visible destination image
        vkcpp::CommandBuffers copy_cmd = std::move(vkcpp::CommandBuffers::beginSingleTimeCmd(device, command_pool));

        // Transition destination image to transfer destination layout
        vkcpp::CommandBuffers::cmdBufferMemoryBarrier(
            copy_cmd[0],
            dst_buffer,
            0,
            size,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT);

        // Transition swapchain image from present to transfer source layout
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            src_image,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        vkcpp::CommandBuffers::cmdCopyImageToBuffer(
            copy_cmd[0],
            dst_buffer,
            src_image,
            {0, 0, 0},
            extent);

        vkcpp::CommandBuffers::cmdBufferMemoryBarrier(
            copy_cmd[0],
            dst_buffer,
            0,
            size,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT);

        // Transition back the swap chain image after the blit is done
        vkcpp::CommandBuffers::cmdImageMemoryBarrier(
            copy_cmd[0],
            src_image,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

        copy_cmd.flush_command_buffer(0);

        // Map image memory so we can start copying from it
        const char *data;
        vkMapMemory(*device, dst_memory, 0, VK_WHOLE_SIZE, 0, (void **)&data);

        return {dst_buffer, dst_memory, data, extent.width * 4};
    }
    void Object2D::unmap_buffer_memory(VkBuffer buffer, VkDeviceMemory memory)
    {
        vkUnmapMemory(*device_, memory);
        vkFreeMemory(*device_, memory, nullptr);
        vkDestroyBuffer(*device_, buffer, nullptr);
    }
}
