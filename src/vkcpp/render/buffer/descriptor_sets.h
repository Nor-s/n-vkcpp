#ifndef VKCPP_RENDER_BUFFER_DESCRIPTOR_SET_H
#define VKCPP_RENDER_BUFFER_DESCRIPTOR_SET_H

#include "vulkan_header.h"

namespace vkcpp
{
    class Device;
    /**
     * layouts_ : point same layout. therefore destroy one layout.
     */
    class DescriptorSets
    {
    protected:
        const Device *device_{nullptr};

        std::vector<VkDescriptorSetLayoutBinding> layout_bindings_;

        std::vector<VkDescriptorSetLayout> layouts_;

        std::vector<VkDescriptorSet> descriptor_sets_;

        VkDescriptorPool pool_{VK_NULL_HANDLE};

        uint32_t size_{0};

    public:
        DescriptorSets(const Device *device, uint32_t size);

        virtual ~DescriptorSets();

        const std::vector<VkDescriptorSet> &get_sets() const { return descriptor_sets_; }

        const std::vector<VkDescriptorSetLayout> &get_layouts() const { return layouts_; }

        void init_layout_bindings();

        void init_layout();

        void init_pool();

        void init_descriptor_sets();

        void destroy_layout();

        void destroy_pool();

    }; // class

} // namespace vkcpp

#endif // #define VKCPP_RENDER_BUFFER_DESCRIPTOR_SET_H