#ifndef CLASS_BRUSH_H
#define CLASS_BRUSH_H

#include "vkcpp.h"

namespace painting
{
    struct BrushAttributeComponent
    {
        glm::vec2 scale_range_{0.01f, 0.1f};
        // for transform
        glm::vec3 scale{0.1f, 0.1f, 1.0f};
        glm::vec3 translation{0.0f, 0.0f, 0.0f};
        float rotation_z{0.0f};
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        int object_idx{0};
        BrushAttributeComponent(const glm::vec2 &scale_range)
            : scale_range_(scale_range)
        {
        }
    }; // struct BrushAttributeComponent

    class Brushes
    {
    public:
        static const int TEX_SIZE_ = 4;

    private:
        const char *tex_[4] = {
            "../../../resources/textures/brushes/4.png",
            "../../../resources/textures/brushes/1.png",
            "../../../resources/textures/brushes/6.png",
            "../../../resources/textures/brushes/9.png"};
        //   brushes_[0].push_back(std::make_unique<Brush>(device_, render_stage_, command_pool_, 0));
        std::vector<std::unique_ptr<vkcpp::Object2D>> brushes_;

    public:
        Brushes(const vkcpp::Device *device,
                const vkcpp::RenderStage *render_stage,
                const vkcpp::CommandPool *command_pool,
                int brush_count);
        const int get_brushes_size() const
        {
            return brushes_.size();
        }
        void draw_all(VkCommandBuffer command_buffer, int ubo_idx);
        void update(const BrushAttributeComponent &attribute, const vkcpp::Camera *camera, int idx, int ubo_idx);
    }; // class Brushes

    class BrushAttributes
    {
    public:
        struct Probablity
        {
            float scale{0.1f};
            float trans{0.1f};
            float rotate{0.1f};
            float color{0.1f};
            Probablity() = default;
            Probablity(float s, float t, float r, float c)
                : scale(s), trans(t), rotate(r), color(c)
            {
            }
        };
        const float max_scale_ = 0.1f;
        const float min_scale_ = 0.02f;

    private:
        std::vector<BrushAttributeComponent> attributes_;

        // for generate range (translation)
        glm::vec2 offset_{};
        glm::vec2 extent_{};
        double fitness_{0.0};
        Probablity probablity_;

    public:
        BrushAttributes(const glm::vec2 &offset,
                        const glm::vec2 &extent,
                        const glm::vec2 &scale_range,
                        int size,
                        const Probablity &p);

        BrushAttributes(const BrushAttributes &a, const BrushAttributes &b);

        ~BrushAttributes();

        const BrushAttributeComponent &get_attribute(int idx) const
        {
            return attributes_[idx];
        }

        bool operator<(BrushAttributes &a)
        {
            return fitness_ < a.fitness_;
        }
        double &get_mutable_fitness()
        {
            return fitness_;
        }
        const double &get_fitness() const
        {
            return fitness_;
        }
        std::unique_ptr<BrushAttributes> cross_over(const BrushAttributes &a);

        void mutate(int idx);

        void set_rand_obj_idx(int idx);
        void set_rand_scale(int idx, bool is_relative = false);
        void set_rand_translation(int idx, bool is_relative = false);
        void set_rand_rotation(int idx, bool is_relative = false);
        void set_rand_color(int idx, bool is_relative = false);
    }; // class BrushAttributes
}
#endif
