#!/bin/bash
cd lib
files=$(ls)
for filename in $files
do
    OLD_IFS="$IFS"
    IFS="."
    arr=($filename)
    IFS="$OLD_IFS"
    arr_len=${#arr[@]}
    if [ $arr_len -ge 4 ]
    then
        new_name=${arr[0]}"."${arr[1]}"."${arr[2]}
        ln -s "${filename}" "${new_name}" >> /dev/null 2>&1
        new_name=${arr[0]}"."${arr[1]}
        ln -s "${filename}" "${new_name}" >> /dev/null 2>&1
    elif [ $arr_len -ge 3 ]
    then
        new_name=${arr[0]}"."${arr[1]}
        ln -s "${filename}" "${new_name}" >> /dev/null 2>&1
    fi
done
exit 0
