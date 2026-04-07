file(GLOB_RECURSE headers CONFIGURE_DEPENDS "*.hpp" "*.h")
target_sources(
    mb.alloc-statistics
    ${_mb_alloc_statistics_fs_scope}
    FILE_SET HEADERS
    FILES
    ${headers}
)
