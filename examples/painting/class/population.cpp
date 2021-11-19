#include "population.h"
#include "utility/utility.h"

namespace painting
{
    Population::Population(const glm::vec2 &offset,
                           const glm::vec2 &extent,
                           const glm::vec2 &scale_range,
                           const BrushAttributes::Probablity &probablity,
                           uint32_t min_population_size,
                           uint32_t attributes_size)
        : scale_range_(scale_range), probablity_(probablity)
    {
        component_.offset = offset;
        component_.extent = extent;

        component_.min_population_size = min_population_size;
        component_.attributes_size = attributes_size;
        component_.stage_count = 0;
        push_back(component_.min_population_size);
    }
    Population::~Population()
    {
        int size = population_.size();
        for (int i = 0; i < size; i++)
        {
            population_[i].reset();
        }
        population_.resize(0);
    }
    void Population::sort()
    {
        std::sort(
            population_.begin(),
            population_.end(),
            [](const std::unique_ptr<BrushAttributes> &a, const std::unique_ptr<BrushAttributes> &b) -> bool
            {
                return a->get_fitness() > b->get_fitness();
            });
    }
    void Population::push_back(int count)
    {
        for (int i = 0; i < count; i++)
        {
            population_.push_back(std::make_unique<BrushAttributes>(
                component_.offset,
                component_.extent,
                scale_range_,
                component_.attributes_size,
                probablity_));
        }
    }
    void Population::pop_back()
    {
        if (population_.size() == 0)
        {
            return;
        }
        population_.back().reset();
        population_.pop_back();
    }
    void Population::next_stage()
    {
        int size = population_.size();
        component_.stage_count++;
        int half_size = size / 2;

        for (int i = 0; i < half_size; i++)
        {
            pop_back();
        }
        for (int i = 0; i < half_size; i++)
        {
            int parent1 = rand() % 3;
            int parent2 = rand() % 3;
            population_.push_back(population_[parent1]->cross_over(*population_[parent2]));
            for (uint32_t j = 0; j < component_.attributes_size; j++)
            {
                if (rand() % 2 == 0)
                {
                    population_.back()->mutate(j);
                }
            }
        }
    }
} // namespace vkcpp