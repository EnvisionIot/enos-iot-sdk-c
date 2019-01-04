SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

FOREACH(subdir ${SUBDIRS})
#    message("${subdir}")
    IF(NOT subdir MATCHES "CMakeFiles")
        ADD_SUBDIRECTORY(${subdir})
    ENDIF()
ENDFOREACH()
