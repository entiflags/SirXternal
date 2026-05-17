add_executable(sir_xternal
    main.cpp
    ../sdk/schema.cpp
    ../sdk/vector.cpp
    ../sdk/matrix.cpp
    ../hooks/present_hook.cpp
    ../overlay/headless_renderer.cpp

    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_dx11.cpp
)

target_include_directories(sir_xternal PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

target_link_libraries(sir_xternal PRIVATE
    liquid_hook_ex
    d3d11
    dxgi
    d3dcompiler
)
