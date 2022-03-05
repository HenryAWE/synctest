cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    message(STATUS "Use vcpkg toolchain from $ENV{VCPKG_ROOT}")
endif()

# Compiler options
# Use C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Force UTF-8 on MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
# Make sure macro "__cplusplus" is correctly defined on MSVC
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

project(sdltemplate)

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
add_subdirectory(imgui)
add_subdirectory(stb)

aux_source_directory(. sdltemplate_src)

add_executable(sdlapp WIN32 ${sdltemplate_src})

target_compile_definitions(sdlapp PUBLIC BOOST_ASIO_SEPARATE_COMPILATION)

target_link_libraries(sdlapp PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(sdlapp PRIVATE Threads::Threads)
target_link_libraries(sdlapp PRIVATE Boost::system)
target_link_libraries(sdlapp PRIVATE imgui)
target_link_libraries(sdlapp PRIVATE stb)
