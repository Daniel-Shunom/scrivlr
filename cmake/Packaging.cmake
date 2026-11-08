install(TARGETS scrivlr RUNTIME DESTINATION . BUNDLE DESTINATION . COMPONENT Runtime)
install(FILES
    "${glfw_SOURCE_DIR}/LICENSE.md"
    DESTINATION licenses/glfw COMPONENT Runtime)
install(FILES
    "${PROJECT_SOURCE_DIR}/vendor/stb/stb_image.h"
    "${PROJECT_SOURCE_DIR}/vendor/stb/stb_truetype.h"
    DESTINATION licenses/stb COMPONENT Runtime)
install(FILES "${PROJECT_SOURCE_DIR}/vendor/clay.h"
    DESTINATION licenses/clay COMPONENT Runtime)
install(FILES "${PROJECT_SOURCE_DIR}/assets/fonts/OFL.txt"
    DESTINATION licenses/NotoSans COMPONENT Runtime)
install(FILES
    "${PROJECT_SOURCE_DIR}/vendor/nanosvg/nanosvg.h"
    "${PROJECT_SOURCE_DIR}/vendor/nanosvg/nanosvgrast.h"
    DESTINATION licenses/nanosvg COMPONENT Runtime)
install(FILES
    "${PROJECT_SOURCE_DIR}/vendor/glad/include/glad/glad.h"
    "${PROJECT_SOURCE_DIR}/vendor/glad/include/KHR/khrplatform.h"
    DESTINATION licenses/glad COMPONENT Runtime)

if(APPLE)
    install(CODE "set(SCRIVLR_RUNTIME_DIRS \"${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}\")" COMPONENT Runtime)
    install(CODE [[
        include(BundleUtilities)
        set(bundle "${CMAKE_INSTALL_PREFIX}/scrivlr.app")
        fixup_bundle("${bundle}" "" "${SCRIVLR_RUNTIME_DIRS}")
        execute_process(COMMAND /usr/bin/codesign --force --deep --sign - "${bundle}"
            COMMAND_ERROR_IS_FATAL ANY)
    ]] COMPONENT Runtime)
endif()

set(CPACK_PACKAGE_NAME "scrivlr")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/packages")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_COMPONENTS_ALL Runtime)
set(CPACK_ARCHIVE_COMPONENT_INSTALL OFF)
if(WIN32 OR APPLE)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()
set(SCRIVLR_PACKAGE_NAME "scrivlr-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
    CACHE STRING "Release archive name without extension")
set(CPACK_PACKAGE_FILE_NAME "${SCRIVLR_PACKAGE_NAME}")
include(CPack)
