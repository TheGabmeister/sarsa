#include <sarsa/fixed_timestep.h>

#include <algorithm>

namespace sarsa {

FixedTimestep::FixedTimestep(FixedTimestepConfig config)
    : m_config(config) {}

void FixedTimestep::reset() {
    m_accumulator_seconds = 0.0;
}

FixedTimestepAdvanceResult FixedTimestep::advance(double frame_delta_seconds) {
    FixedTimestepAdvanceResult result{};
    result.step_delta_seconds = static_cast<float>(m_config.fixed_delta_seconds);

    result.clamped_frame_delta_seconds = std::clamp(
        frame_delta_seconds,
        0.0,
        m_config.max_frame_delta_seconds);

    m_accumulator_seconds += result.clamped_frame_delta_seconds;

    while (m_accumulator_seconds >= m_config.fixed_delta_seconds &&
           result.steps < m_config.max_steps_per_frame) {
        m_accumulator_seconds -= m_config.fixed_delta_seconds;
        ++result.steps;
    }

    if (m_accumulator_seconds < 0.0) {
        m_accumulator_seconds = 0.0;
    }

    result.accumulator_seconds = m_accumulator_seconds;
    result.interpolation_alpha =
        m_accumulator_seconds / m_config.fixed_delta_seconds;

    return result;
}

} // namespace sarsa
