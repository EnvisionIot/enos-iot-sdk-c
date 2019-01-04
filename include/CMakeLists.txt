SET(INCLUDE_COPY_NAME "enos_c_api")

add_custom_target(cp_include ALL)

IF(OS_DEFINE STREQUAL "linux")
    add_custom_command(TARGET cp_include
        PRE_BUILD
        COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/include/${INCLUDE_COPY_NAME}"
        COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_COPY_NAME}" "${PROJECT_SOURCE_DIR}/dst/include/"
        COMMENT "cp include to dst(${OS_DEFINE})"
    )
ELSEIF(OS_DEFINE STREQUAL "windows")
    STRING(REGEX REPLACE "/" "\\\\" PROJECT_SOURCE_DIR_NATIVE ${PROJECT_SOURCE_DIR} )
    STRING(REGEX REPLACE "/" "\\\\" CMAKE_CURRENT_SOURCE_DIR_NATIVE ${CMAKE_CURRENT_SOURCE_DIR} )
    IF(SH_TYPE STREQUAL "sh")
        IF(CMAKE_SH STREQUAL "CMAKE_SH-NOTFOUND")
            add_custom_command(TARGET cp_include
                PRE_BUILD
                COMMAND if exist "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\${INCLUDE_COPY_NAME}" rd /s/q "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\${INCLUDE_COPY_NAME}"
                COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\${INCLUDE_COPY_NAME}" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\${INCLUDE_COPY_NAME}" /y /i /s /e /h
                COMMENT "cp include to dst(${OS_DEFINE}) (${CMAKE_CURRENT_SOURCE_DIR_NATIVE})"
            )
        ELSE()
            add_custom_command(TARGET cp_include
                PRE_BUILD
                COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/include/${INCLUDE_COPY_NAME}"
                COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_COPY_NAME}" "${PROJECT_SOURCE_DIR}/dst/include/"
                COMMENT "cp include to dst(${OS_DEFINE}) (${CMAKE_CURRENT_SOURCE_DIR_NATIVE})"
            )
        ENDIF()
    ELSE()
        add_custom_command(TARGET cp_include
            PRE_BUILD
            COMMAND if exist "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\${INCLUDE_COPY_NAME}" rd /s/q "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\${INCLUDE_COPY_NAME}"
            COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\${INCLUDE_COPY_NAME}" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\${INCLUDE_COPY_NAME}" /y /i /s /e /h
            COMMENT "cp include to dst(${OS_DEFINE}) (${CMAKE_CURRENT_SOURCE_DIR_NATIVE})"
        )
    ENDIF()
ELSE()
    add_custom_command(TARGET cp_include
        PRE_BUILD
        COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/include/${INCLUDE_COPY_NAME}"
        COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_COPY_NAME}" "${PROJECT_SOURCE_DIR}/dst/include/"
        COMMENT "cp include to dst(${OS_DEFINE})"
    )
ENDIF()
