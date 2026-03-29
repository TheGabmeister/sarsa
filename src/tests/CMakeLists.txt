add_executable(sarsa_tests
    test_main.cpp
    log_tests.cpp
    assert_tests.cpp
    fixed_timestep_tests.cpp
    memory_diagnostics_tests.cpp
)

target_include_directories(sarsa_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sarsa_tests
    PRIVATE sarsa_engine
)

sarsa_set_compiler_warnings(sarsa_tests)
sarsa_disable_exceptions(sarsa_tests)

add_test(NAME sarsa_tests COMMAND sarsa_tests)
