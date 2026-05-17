add_executable(sir_xternal
    main.cpp
    ../sdk/schema.cpp
)

target_include_directories(sir_xternal PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(sir_xternal PRIVATE
    liquid_hook_ex
)
