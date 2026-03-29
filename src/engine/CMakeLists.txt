add_library(sarsa_engine STATIC
    engine.cpp
    log.cpp
    assert.cpp
    crash_handler.cpp
    fixed_timestep.cpp
)

target_include_directories(sarsa_engine
    PUBLIC include
)

target_link_libraries(sarsa_engine
    PUBLIC spdlog::spdlog
)

sarsa_set_compiler_warnings(sarsa_engine)
sarsa_disable_exceptions(sarsa_engine)
