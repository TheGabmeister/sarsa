add_library(sarsa_game MODULE
    game_module.cpp
)

target_include_directories(sarsa_game
    PUBLIC include
)

target_link_libraries(sarsa_game
    PRIVATE sarsa_engine
)

sarsa_set_compiler_warnings(sarsa_game)
sarsa_disable_exceptions(sarsa_game)
