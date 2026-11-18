set(ALL_EXAMPLES usage)

foreach(example ${ALL_EXAMPLES})
    add_executable(mb.alloc-statistics.examples.${example})
    target_sources(
        mb.alloc-statistics.examples.${example}
        PRIVATE ${example}.cpp
    )
    target_link_libraries(
        mb.alloc-statistics.examples.${example}
        PRIVATE mb::alloc-statistics
    )
    target_compile_features(
        mb.alloc-statistics.examples.${example}
        PRIVATE cxx_std_20
    )
endforeach()
