function(sarsa_set_compiler_warnings target_name)
    set(MSVC_WARNINGS
        /W4             # highest practical warning level (covers most of -Wall -Wextra)
        /WX             # treat warnings as errors
        /permissive-    # strict C++ conformance, reject MSVC extensions
        /w14640         # warn on thread-unsafe static local initialization
        /w14826         # warn on narrowing conversions between signed/unsigned
        /w14928         # warn on more than one implicit user-defined conversion in copy-init
    )

    set(CLANG_WARNINGS
        -Wall               # broad set of common warnings
        -Wextra             # additional warnings not covered by -Wall
        -Wpedantic          # warn on non-standard C++ usage
        -Werror             # treat warnings as errors
        -Wconversion        # warn on implicit type conversions that may lose data
        -Wsign-conversion   # warn on implicit signed/unsigned conversions
        -Wshadow            # warn when a local variable shadows an outer one
        -Wnon-virtual-dtor  # warn on classes with virtual methods but non-virtual destructor
        -Wold-style-cast    # warn on C-style casts, prefer static_cast/reinterpret_cast
        -Wcast-align        # warn on casts that increase required alignment
        -Woverloaded-virtual  # warn when a derived class hides a base class virtual method
        -Wnull-dereference  # warn on code paths that dereference null pointers
        -Wformat=2          # extra format string checks (security-relevant)
    )

    set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation  # warn when indentation doesn't match block structure
        -Wduplicated-cond         # warn on if/else chains with duplicated conditions
        -Wduplicated-branches     # warn on if/else branches with identical bodies
        -Wlogical-op              # warn on suspicious uses of logical operators
        -Wuseless-cast            # warn on casts to the same type
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target_name} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target_name} PRIVATE ${GCC_WARNINGS})
    endif()
endfunction()

function(sarsa_disable_exceptions target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /EHs-c-)
        target_compile_definitions(${target_name} PRIVATE _HAS_EXCEPTIONS=0)
    else()
        target_compile_options(${target_name} PRIVATE -fno-exceptions)
    endif()
endfunction()
