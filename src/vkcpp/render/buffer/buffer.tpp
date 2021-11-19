namespace vkcpp
{
    template <typename T>
    Buffer<T>::Buffer(const Device *device,
                      const CommandPool *command_pool,
                      std::vector<T> *src_data,
                      VkBufferUsageFlagBits usage,
                      bool is_local)
        : device_(device),
          command_pool_(command_pool),
          src_data_(src_data),
          usage_(usage)
    {
        init_buffer(is_local);
    }

    template <typename T>
    Buffer<T>::Buffer(Buffer &&a)
        : device_(a.device_), command_pool_(a.command_pool_), src_data_(a.src_data_), handle_(a.handle_), memory_(a.memory_), usage_(std::move(a.usage_))
    {
        a.src_data_ = nullptr;
        a.handle_ = VK_NULL_HANDLE;
        a.memory_ = VK_NULL_HANDLE;
    }

    template <typename T>
    Buffer<T>::~Buffer()
    {
        destroy_buffer();
        free_memory();
    }

    template <typename T>
    Buffer<T> &Buffer<T>::operator=(Buffer &&a)
    {
        if (this != &a)
        {
            device_ = a.device_;
            command_pool_ = a.command_pool_;
            src_data_ = a.src_data_;
            handle_ = a.handle_;
            memory_ = a.memory_;
            usage_ = std::move(a.usage_);

            a.src_data_ = nullptr;
            a.handle_ = VK_NULL_HANDLE;
            a.memory_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    template <typename T>
    Buffer<T>::operator const VkBuffer &() const { return handle_; }

    template <typename T>
    const VkBuffer &Buffer<T>::get_buffer() const { return handle_; }

    template <typename T>
    const VkMemoryPropertyFlags &Buffer<T>::get_memory_property() const { return memory_property_; }

    template <typename T>
    VkBuffer &Buffer<T>::get_mutable_buffer() { return handle_; }

    template <typename T>
    VkDeviceMemory &Buffer<T>::get_mutable_memory() { return memory_; }

    template <typename T>
    void Buffer<T>::init_buffer(bool is_local)
    {
        if (is_local)
        {
            create_local_memory();
        }
        else
        {
            create_host_memory();
        }
    }

    template <typename T>
    void Buffer<T>::destroy_buffer()
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(*device_, handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }

    template <typename T>
    void Buffer<T>::free_memory()
    {
        if (memory_ != VK_NULL_HANDLE)
        {
            vkFreeMemory(*device_, memory_, nullptr);
            memory_ = VK_NULL_HANDLE;
        }
    }

    template <typename T>
    void Buffer<T>::create_host_memory()
    {
        memory_property_ = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkDeviceSize buffer_size;
        if (src_data_ != nullptr)
        {
            buffer_size = sizeof((*src_data_)[0]) * (*src_data_).size();
        }
        else // for ubo
        {
            buffer_size = sizeof(T);
        }

        create::buffer(device_,
                       buffer_size,
                       usage_,
                       memory_property_,
                       handle_,
                       memory_);

        if (src_data_ != nullptr)
        {
            void *dst_data;
            vkMapMemory(*device_, memory_, 0, buffer_size, 0, &dst_data);
            memcpy(dst_data, (*src_data_).data(), (size_t)buffer_size);
            vkUnmapMemory(*device_, memory_);
        }
    }

    template <typename T>
    void Buffer<T>::create_local_memory()
    {
        memory_property_ = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VkDeviceSize buffer_size = sizeof((*src_data_)[0]) * (*src_data_).size();
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        create::buffer(device_,
                       buffer_size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       staging_buffer,
                       staging_buffer_memory);

        void *dst_data;
        vkMapMemory(*device_, staging_buffer_memory, 0, buffer_size, 0, &dst_data);
        memcpy(dst_data, (*src_data_).data(), (size_t)buffer_size);
        vkUnmapMemory(*device_, staging_buffer_memory);

        create::buffer(device_,
                       buffer_size,
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage_,
                       memory_property_,
                       handle_,
                       memory_);

        CommandBuffers::cmdSingleCopyBuffer(device_, command_pool_, staging_buffer, handle_, buffer_size);

        vkDestroyBuffer(*device_, staging_buffer, nullptr);
        vkFreeMemory(*device_, staging_buffer_memory, nullptr);
    }

} // namespace vkcpp
