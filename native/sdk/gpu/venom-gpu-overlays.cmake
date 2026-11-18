target_include_directories(rexgpu-xenos BEFORE PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include-overlay-0"
    "${CMAKE_CURRENT_LIST_DIR}/include-overlay-1"
    "${CMAKE_CURRENT_LIST_DIR}/include-overlay-2"
    "${CMAKE_CURRENT_LIST_DIR}/include-overlay-3")
set_source_files_properties(d3d12/render_target_cache.cpp PROPERTIES INCLUDE_DIRECTORIES
    "${CMAKE_CURRENT_LIST_DIR}/per-file-include-0;${CMAKE_CURRENT_LIST_DIR}/per-file-include-1;${CMAKE_CURRENT_LIST_DIR}/per-file-include-2")
set_source_files_properties(d3d12/texture_cache.cpp PROPERTIES INCLUDE_DIRECTORIES
    "${CMAKE_CURRENT_LIST_DIR}/per-file-include-0")
