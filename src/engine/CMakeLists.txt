add_library(sarsa_engine STATIC
    engine.cpp
)

target_include_directories(sarsa_engine
    PUBLIC include
)

sarsa_set_compiler_warnings(sarsa_engine)
sarsa_disable_exceptions(sarsa_engine)
