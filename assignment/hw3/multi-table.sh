#!/usr/bin/bash
if [ "$*" == "" ]; then
    echo "No arguments provided"
    exit 1
fi

for i in "$@"
do
    case "${i#[-+]}" in
        *[!0-9]* | '' | 0)
            echo positive integer only
            exit 1;;
    esac
done


for i in `seq 1 $1`
do
	for j in `seq 1 $2`
	do
		mul=`expr $i \* $j`
		echo -n "${i}*${j}=${mul} "

	done
echo ""
done

exit 0
