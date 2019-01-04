#set(LOG "log.txt")
#add_custom_command(OUTPUT  ${LOG}
#  COMMAND echo "Generating log.txt file..."
#  COMMAND echo "welcome"
#  COMMENT  "This is a test"
# )

#add_custom_target(cp_support_lib ALL DEPENDS ${LOG})
add_custom_target(cp_support_lib ALL)

IF(OS_DEFINE STREQUAL "linux")
    add_custom_command(TARGET cp_support_lib
        PRE_BUILD
        COMMAND chmod 755 ./generate_soft_link.sh
        COMMAND ./generate_soft_link.sh
        COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/lib/support"
        COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/dst/lib/support"
        COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/include/support"
        COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/dst/include/support"
        COMMENT "cp support include and lib to dst(${OS_DEFINE})"
    )
ELSEIF(OS_DEFINE STREQUAL "windows")
    STRING(REGEX REPLACE "/" "\\\\" PROJECT_SOURCE_DIR_NATIVE ${PROJECT_SOURCE_DIR} )
    STRING(REGEX REPLACE "/" "\\\\" CMAKE_CURRENT_SOURCE_DIR_NATIVE ${CMAKE_CURRENT_SOURCE_DIR} )
    IF(SH_TYPE STREQUAL "sh")
        IF(CMAKE_SH STREQUAL "CMAKE_SH-NOTFOUND")
            add_custom_command(TARGET cp_support_lib
                PRE_BUILD
                COMMAND if exist "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" rd /s/q "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support"
                COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\lib" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" /y /i /s /e /h
                COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\dll\\*" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" /y /i /s /e /h
                #COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\static\\*.a" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" /y /i /s /e /h
                COMMAND if exist "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\support" rd /s/q "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\support"
                COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\include" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\support" /y /i /s /e /h
                COMMENT "cp support include and lib to dst(${OS_DEFINE}) (${CMAKE_CURRENT_SOURCE_DIR_NATIVE})"
            )
        ELSE()
            add_custom_command(TARGET cp_support_lib
                PRE_BUILD
                COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/lib/support"
                COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/dst/lib/support"
                COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/dll/*" "${PROJECT_SOURCE_DIR}/dst/lib/support"
                #COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/static/*.a" "${PROJECT_SOURCE_DIR}/dst/lib/support"
                COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/include/support"
                COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/dst/include/support"
                COMMENT "cp support include and lib to dst(${OS_DEFINE}) (${CMAKE_CURRENT_SOURCE_DIR_NATIVE})"
            )
        ENDIF()
    ELSE()
        add_custom_command(TARGET cp_support_lib
            PRE_BUILD
            COMMAND if exist "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" rd /s/q "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support"
            COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\lib" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" /y /i /s /e /h
            COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\dll\\*.dll" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" /y /i /s /e /h
            #COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\static\\*.a" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\lib\\support" /y /i /s /e /h
            COMMAND if exist "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\support" rd /s/q "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\support"
            COMMAND xcopy "${CMAKE_CURRENT_SOURCE_DIR_NATIVE}\\include" "${PROJECT_SOURCE_DIR_NATIVE}\\dst\\include\\support" /y /i /s /e /h
            COMMENT "cp support include and lib to dst(${OS_DEFINE}) (${CMAKE_CURRENT_SOURCE_DIR_NATIVE})"
        )
    ENDIF()
ELSE()
    add_custom_command(TARGET cp_support_lib
        PRE_BUILD
        COMMAND chmod 755 ./generate_soft_link.sh
        COMMAND ./generate_soft_link.sh
        COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/lib/support"
        COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/lib" "${PROJECT_SOURCE_DIR}/dst/lib/support"
        COMMAND rm -rf "${PROJECT_SOURCE_DIR}/dst/include/support"
        COMMAND cp -r -d "${CMAKE_CURRENT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/dst/include/support"
        COMMENT "cp support include and lib to dst(${OS_DEFINE})"
    )
ENDIF()
