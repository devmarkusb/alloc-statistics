file(GLOB_RECURSE headers CONFIGURE_DEPENDS "*.hpp" "*.h")

target_sources(
    mb.alloc-statistics
    PUBLIC FILE_SET HEADERS FILES ${headers}
)
