#include "test_framework.h"

#include <sarsa/fixed_timestep.h>

#include <cmath>

namespace {

bool nearly_equal(double left, double right, double epsilon = 1.0e-9) {
    return std::fabs(left - right) <= epsilon;
}

} // namespace

SR_TEST(fixed_timestep_accumulates_partial_frame_time) {
    sarsa::FixedTimestep timestep;

    const auto first = timestep.advance(1.0 / 120.0);
    SR_TEST_CHECK(first.steps == 0);
    SR_TEST_CHECK(nearly_equal(first.interpolation_alpha, 0.5));

    const auto second = timestep.advance(1.0 / 120.0);
    SR_TEST_CHECK(second.steps == 1);
    SR_TEST_CHECK(nearly_equal(second.accumulator_seconds, 0.0));
    SR_TEST_CHECK(nearly_equal(second.interpolation_alpha, 0.0));
}

SR_TEST(fixed_timestep_clamps_large_frame_spikes) {
    sarsa::FixedTimestep timestep(sarsa::FixedTimestepConfig{
        .fixed_delta_seconds = 1.0 / 60.0,
        .max_frame_delta_seconds = 0.25,
        .max_steps_per_frame = 32,
    });

    const auto result = timestep.advance(1.0);

    SR_TEST_CHECK(result.steps == 15);
    SR_TEST_CHECK(nearly_equal(result.clamped_frame_delta_seconds, 0.25));
    SR_TEST_CHECK(nearly_equal(result.accumulator_seconds, 0.0));
}

SR_TEST(fixed_timestep_limits_steps_per_frame_and_preserves_backlog) {
    sarsa::FixedTimestep timestep(sarsa::FixedTimestepConfig{
        .fixed_delta_seconds = 1.0 / 60.0,
        .max_frame_delta_seconds = 0.25,
        .max_steps_per_frame = 4,
    });

    const auto result = timestep.advance(0.10);

    SR_TEST_CHECK(result.steps == 4);
    SR_TEST_CHECK(nearly_equal(
        result.accumulator_seconds,
        0.10 - (4.0 * (1.0 / 60.0))));
}
