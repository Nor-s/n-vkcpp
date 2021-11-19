

namespace vkcpp
{
    template <typename T>
    UniformBuffers<T>::UniformBuffers(const Device *device_, const Image *image, uint32_t size)
        : DescriptorSets(device_, size), image_(image)
    {
        init_uniform_buffers();
        update_descriptor();
    }

    template <typename T>
    UniformBuffers<T>::~UniformBuffers()
    {
        destroy_uniform_buffers();
    }

    template <typename T>
    void UniformBuffers<T>::init_uniform_buffers()
    {
        if (handle_.size() != 0)
        {
            destroy_uniform_buffers();
        }
        for (uint32_t i = 0; i < size_; i++)
        {
            handle_.emplace_back(
                Buffer<T>(
                    device_,
                    nullptr,
                    nullptr,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    false));
        }
    }

    template <typename T>
    void UniformBuffers<T>::destroy_uniform_buffers()
    {
        for (auto &ubo : handle_)
        {
            ubo.destroy_buffer();
        }
        handle_.resize(0);
        size_ = 0;
    }

    template <typename T>
    void UniformBuffers<T>::update_uniform_buffer(uint32_t idx, const T &src_data)
    {
        if (idx >= size_)
        {
            throw std::runtime_error("failed to update ubo! out of bounds");
        }
        void *dst_data;
        vkMapMemory(*device_, handle_[idx].get_mutable_memory(), 0, sizeof(T), 0, &dst_data);
        memcpy(dst_data, &src_data, sizeof(src_data));
        // TODO : Buffer 클래스 local,  host&coherent 와 host&cached 선택 가능하게., 만일 flush 할경우 메모리 크기가 VkPhysicalDeviceLimits::nonCoherentAtomSize 의 배수여야함.
        //  If host coherency hasn't been requested, do a manual flush to make writes visible

        /*
     if ((handle_[idx].get_memory_property() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
    VkMappedMemoryRange mapped_range{};
    mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = handle_[idx].get_mutable_memory();
    mapped_range.offset = 0;
    mapped_range.size = VK_WHOLE_SIZE;
    vkFlushMappedMemoryRanges(*device_, 1, &mapped_range);
    }
    */
        vkUnmapMemory(*device_, handle_[idx].get_mutable_memory());
    }
    template <typename T>
    void UniformBuffers<T>::update_descriptor(int i)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = handle_[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(T); // or VK_WHOLE_SIZE

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = get_image_view();
        image_info.sampler = get_sampler();

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptor_sets_[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;
        descriptor_writes[0].pImageInfo = nullptr;       // Optional
        descriptor_writes[0].pTexelBufferView = nullptr; // Optional

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptor_sets_[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = nullptr; // Optional
        descriptor_writes[1].pImageInfo = &image_info;
        descriptor_writes[1].pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(*device_, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
    }
    template <typename T>
    void UniformBuffers<T>::update_descriptor()
    {
        for (size_t i = 0; i < size_; i++)
        {
            update_descriptor(i);
        }
    }

} // namespace vkcpp