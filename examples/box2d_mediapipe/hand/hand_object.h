#ifndef __HAND__HAND_ObJECT__
#define __HAND__HAND_ObJECT__

#include "vkcpp.h"
#include "hand_input.h"
#include "box_object.h"

// 0-1 1-2 2-3 3-4
// 0-5 5-6 6-7 7-8
// 5-9 9-10 10-11 11-12
// 9-13 13-14 14-15 15-16
// 13-17 17-18 18-19 19-20
// 0-17
/**
 *
 */
namespace hand
{
    class HandObject
    {
    private:
        static const std::vector<std::pair<int, int>> landmark_pair_;
        static const HandInput hand_input_;
        static const float hand_scale_;
        // ubos_.size() ==  21 * ubosize;
        std::vector<std::unique_ptr<box::BoxObject>> hand_seg_;
        int height_;
        int width_;

    public:
        HandObject(const vkcpp::Device *device,
                   const vkcpp::RenderStage *render_stage,
                   const vkcpp::CommandPool *command_pool);
        virtual ~HandObject();

        void draw_all(VkCommandBuffer command_buffer, int ubo_idx);
        glm::vec3 cal_scale(int idx, const std::pair<glm::vec3, glm::vec3> &p);
        glm::vec3 cal_trans(int idx, const std::pair<glm::vec3, glm::vec3> &p);
        glm::vec3 cal_rotate(int idx, const std::pair<glm::vec3, glm::vec3> &p);
        glm::vec3 resize(const glm::vec3 &center, const glm::vec3 &p, float hand_scale);
        void update_all(int ubo_idx);
    };
}
#endif