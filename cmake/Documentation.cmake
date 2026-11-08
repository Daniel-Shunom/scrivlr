option(SCRIVLR_BUILD_DOCS "Enable the Doxygen documentation target" OFF)

if(SCRIVLR_BUILD_DOCS)
    find_package(Doxygen REQUIRED COMPONENTS doxygen)
    doxygen_add_docs(docs
        CONFIG_FILE "${PROJECT_SOURCE_DIR}/Doxyfile"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Generating scrivlr API reference and guides"
    )
endif()
