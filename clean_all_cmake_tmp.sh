#!/bin/bash

rm_by_name()
{
    if [ $# -ge 1 ]
    then
        find_result=`find ./ -name "$1"`
#        echo "$find_result"
        for element in ${find_result}
        do
            echo "rm -rf ${element}"
            rm -rf "${element}"
        done
    fi
}

rm_by_name "CMakeFiles"
rm_by_name "cmake_install.cmake"
rm_by_name "CMakeCache.txt"
rm_by_name "Makefile"
rm_by_name "*.vcproj"
rm_by_name "*.vcxproj*"
rm_by_name "*.opensdf"
rm_by_name "*.sdf"
rm_by_name "*.suo"
rm_by_name "*.sln"
rm_by_name "*.def"
rm_by_name "*.dir"
rm_by_name "Debug"
rm_by_name "Release"
rm_by_name "Win32"
rm_by_name "Win64"