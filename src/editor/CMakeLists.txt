add_executable(sarsa_editor
    main.cpp
)

target_link_libraries(sarsa_editor
    PRIVATE sarsa_engine
)

sarsa_set_compiler_warnings(sarsa_editor)
sarsa_disable_exceptions(sarsa_editor)
