#ifndef VKCPP_RENDER_BUFFER_BUFFER_HPP
#define VKCPP_RENDER_BUFFER_BUFFER_HPP

#include "vulkan_header.h"

#include "render/command/command_pool.h"
#include "device/device.h"
#include "render/command/command_buffers.h"

#include "utility/create.h"

namespace vkcpp
{
    template <typename T>
    class Buffer
    {
    protected:
        const Device *device_{nullptr};

        const CommandPool *command_pool_{nullptr};

        std::vector<T> *src_data_;

        VkBuffer handle_{VK_NULL_HANDLE};

        VkDeviceMemory memory_{VK_NULL_HANDLE};

        VkBufferUsageFlagBits usage_{};

        VkMemoryPropertyFlags memory_property_{};

    public:
        Buffer() = default;

        Buffer(const Device *device,
               const CommandPool *command_pool,
               std::vector<T> *src_data,
               VkBufferUsageFlagBits usage,
               bool is_local);

        Buffer(const Buffer &) = delete;

        Buffer(Buffer &&a);

        virtual ~Buffer();

        Buffer<T> &operator=(Buffer &&a);

        operator const VkBuffer &() const;

        const VkBuffer &get_buffer() const;

        const VkMemoryPropertyFlags &get_memory_property() const;

        VkBuffer &get_mutable_buffer();

        VkDeviceMemory &get_mutable_memory();

        void init_buffer(bool is_local);

        void destroy_buffer();

        void free_memory();

        void create_host_memory();

        void create_local_memory();
    };
} // namespace vkcpp

#include "buffer.tpp"

#endif // #ifndef VKCPP_RENDER_BUFFER_BUFFER_HPP