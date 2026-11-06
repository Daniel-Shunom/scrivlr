cmake_minimum_required(VERSION 3.30)

project(scrivlr VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE STATIC CACHE STRING "" FORCE)
FetchContent_Declare(glfw
    URL https://codeload.github.com/glfw/glfw/tar.gz/refs/tags/3.4
    URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01
    TIMEOUT 60
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(glfw)

add_library(scrivlr_glad STATIC vendor/glad/src/glad.c)
target_include_directories(scrivlr_glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include")
target_link_libraries(scrivlr_glad PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(scrivlr_glad PUBLIC cxx_std_26)
target_sources(scrivlr_glad PUBLIC FILE_SET CXX_MODULES FILES src/glad.cppm)
add_library(scrivlr::glad ALIAS scrivlr_glad)

add_library(scrivlr_lib STATIC)
add_library(scrivlr::lib ALIAS scrivlr_lib)
target_compile_features(scrivlr_lib PUBLIC cxx_std_26)
target_sources(scrivlr_lib
    PUBLIC
        FILE_SET CXX_MODULES FILES src/scrivlr_lib.cppm
)

# Keep upstream headers separate from project warning and lint policies.
add_library(scrivlr_clay INTERFACE)
add_library(scrivlr::clay ALIAS scrivlr_clay)
target_include_directories(scrivlr_clay SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
target_compile_features(scrivlr_clay INTERFACE cxx_std_26)

add_executable(scrivlr src/main.cpp)
target_link_libraries(scrivlr PRIVATE scrivlr::lib scrivlr::glad glfw)
target_compile_definitions(scrivlr PRIVATE GLFW_INCLUDE_NONE)

include(CTest)
if(BUILD_TESTING)
    add_executable(glad_module_smoke tests/glad_module_smoke.cpp)
    target_link_libraries(glad_module_smoke PRIVATE scrivlr::glad)
    add_test(NAME glad_module_smoke COMMAND glad_module_smoke)
endif()

if(MSVC)
    target_compile_options(scrivlr_lib PRIVATE /W4 /permissive- /Zc:__cplusplus)
    target_compile_options(scrivlr PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(scrivlr_lib PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    target_compile_options(scrivlr PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()
