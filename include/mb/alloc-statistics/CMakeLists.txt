file(GLOB_RECURSE headers CONFIGURE_DEPENDS "*.hpp" "*.h")

target_sources(
    mb.alloc-statistics
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
            FILES ${headers}
)
