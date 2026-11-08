cmake_minimum_required(VERSION 3.30)

project(scrivlr VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    message(FATAL_ERROR "GCC 16+ is required for C++26 reflection. Use build.ps1 with the local toolchain.")
endif()

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-std=c++26 -freflection")
check_cxx_source_compiles("#include <meta>
static_assert(__cpp_impl_reflection >= 202506L);
using reflected_int = [:^^int:];
int main() { reflected_int value = 0; return value; }" SCRIVLR_HAS_REFLECTION)
unset(CMAKE_REQUIRED_FLAGS)
if(NOT SCRIVLR_HAS_REFLECTION)
    message(FATAL_ERROR "This compiler and standard library must support C++26 <meta> and -freflection.")
endif()

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

add_library(scrivlr_stb_image STATIC vendor/stb/stb_image_impl.c)
target_include_directories(scrivlr_stb_image SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb")

add_library(scrivlr_nanosvg STATIC vendor/nanosvg/nanosvg_impl.c)
target_include_directories(scrivlr_nanosvg SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/nanosvg")
if(UNIX)
    target_link_libraries(scrivlr_nanosvg PRIVATE m)
endif()

add_library(scrivlr_glad STATIC vendor/glad/src/glad.c)
target_include_directories(scrivlr_glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include")
target_link_libraries(scrivlr_glad PRIVATE ${CMAKE_DL_LIBS})
target_compile_features(scrivlr_glad PUBLIC cxx_std_26)
target_sources(scrivlr_glad PUBLIC FILE_SET CXX_MODULES FILES src/glad.cppm)
add_library(scrivlr::glad ALIAS scrivlr_glad)

add_library(scrivlr_lib STATIC)
add_library(scrivlr::lib ALIAS scrivlr_lib)
target_compile_features(scrivlr_lib PUBLIC cxx_std_26)
target_link_libraries(scrivlr_lib PRIVATE glfw scrivlr_stb_image scrivlr_nanosvg scrivlr::glad scrivlr_ui_backend)
target_link_libraries(scrivlr_lib PUBLIC scrivlr::clay)
target_compile_definitions(scrivlr_lib PRIVATE GLFW_INCLUDE_NONE)
file(GLOB_RECURSE SCRIVLR_MODULES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm")
list(REMOVE_ITEM SCRIVLR_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.cppm")
target_sources(scrivlr_lib
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ${SCRIVLR_MODULES}
)

add_library(scrivlr_clay INTERFACE)
add_library(scrivlr::clay ALIAS scrivlr_clay)
target_include_directories(scrivlr_clay SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
target_compile_features(scrivlr_clay INTERFACE cxx_std_26)

add_library(scrivlr_ui_backend STATIC src/ui/backend.cpp vendor/stb/stb_truetype_impl.c)
target_compile_features(scrivlr_ui_backend PRIVATE cxx_std_26)
target_link_libraries(scrivlr_ui_backend PRIVATE scrivlr::clay scrivlr::glad)
target_include_directories(scrivlr_ui_backend SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/clay_renderer"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb")

add_executable(scrivlr src/main.cpp)
target_link_libraries(scrivlr PRIVATE scrivlr::lib scrivlr::glad glfw)
target_compile_definitions(scrivlr PRIVATE GLFW_INCLUDE_NONE)

option(SCRIVLR_STATIC_RUNTIME "Link the GNU runtime statically on Windows/Linux" ON)
if(SCRIVLR_STATIC_RUNTIME AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE)
    target_link_options(scrivlr PRIVATE -static-libgcc -static-libstdc++)
    if(MINGW)
        target_link_options(scrivlr PRIVATE -static)
    endif()
endif()
if(APPLE)
    set_target_properties(scrivlr PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.scrivlr.app"
        MACOSX_BUNDLE_BUNDLE_NAME "scrivlr"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(component_registry_test tests/component_registry_test.cpp)
    target_link_libraries(component_registry_test PRIVATE scrivlr::lib)
    add_test(NAME component_registry_test COMMAND component_registry_test)
    add_executable(component_library_test tests/component_library_test.cpp)
    target_link_libraries(component_library_test PRIVATE scrivlr::lib scrivlr::clay)
    add_test(NAME component_library_test COMMAND component_library_test)
    add_library(reflection_fixture STATIC)
    target_compile_features(reflection_fixture PUBLIC cxx_std_26)
    target_compile_options(reflection_fixture PUBLIC -freflection)
    target_sources(reflection_fixture PUBLIC FILE_SET CXX_MODULES FILES tests/reflection_fixture.cppm)
    add_executable(reflection_test tests/reflection_test.cpp)
    target_link_libraries(reflection_test PRIVATE reflection_fixture)
    add_test(NAME reflection_test COMMAND reflection_test)
    add_executable(ui_render_test tests/ui_render_test.cpp)
    target_link_libraries(ui_render_test PRIVATE scrivlr::lib scrivlr::clay scrivlr::glad)
    option(SCRIVLR_GPU_TESTS "Run tests requiring a desktop OpenGL context" OFF)
    if(SCRIVLR_GPU_TESTS)
        add_test(NAME ui_render_test COMMAND ui_render_test)
    endif()
    add_executable(image_test tests/image_test.cpp)
    target_link_libraries(image_test PRIVATE scrivlr::lib)
    add_test(NAME image_test COMMAND image_test)
    set_tests_properties(image_test PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(glad_module_smoke tests/glad_module_smoke.cpp)
    target_link_libraries(glad_module_smoke PRIVATE scrivlr::glad)
    add_test(NAME glad_module_smoke COMMAND glad_module_smoke)
endif()

include(cmake/Packaging.cmake)
include(cmake/Documentation.cmake)

if(MSVC)
    target_compile_options(scrivlr_lib PRIVATE /W4 /permissive- /Zc:__cplusplus)
    target_compile_options(scrivlr PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(scrivlr_lib PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    target_compile_options(scrivlr PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

target_compile_options(scrivlr_lib PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:-freflection>"
)
