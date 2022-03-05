find_package(SDL2 REQUIRED)

aux_source_directory(. imgui_src)

add_library(imgui STATIC ${imgui_src})
target_link_libraries(imgui PUBLIC SDL2::SDL2)
target_include_directories(imgui PUBLIC .)
