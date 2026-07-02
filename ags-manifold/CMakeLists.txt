add_library(ags-manifold INTERFACE)

target_include_directories(ags-manifold
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ags-manifold
        INTERFACE
        glm::glm
)
