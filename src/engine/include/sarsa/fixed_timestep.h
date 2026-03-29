#pragma once

#include <cstdint>

namespace sarsa {

struct FixedTimestepConfig {
    double fixed_delta_seconds = 1.0 / 60.0;
    double max_frame_delta_seconds = 0.25;
    std::uint32_t max_steps_per_frame = 4;
};

struct FixedTimestepAdvanceResult {
    std::uint32_t steps = 0;
    float step_delta_seconds = 0.0F;
    double clamped_frame_delta_seconds = 0.0;
    double accumulator_seconds = 0.0;
    double interpolation_alpha = 0.0;
};

class FixedTimestep {
public:
    explicit FixedTimestep(FixedTimestepConfig config = {});

    [[nodiscard]] const FixedTimestepConfig& config() const { return m_config; }
    [[nodiscard]] double accumulator_seconds() const { return m_accumulator_seconds; }

    void reset();
    [[nodiscard]] FixedTimestepAdvanceResult advance(double frame_delta_seconds);

private:
    FixedTimestepConfig m_config;
    double m_accumulator_seconds = 0.0;
};

} // namespace sarsa
