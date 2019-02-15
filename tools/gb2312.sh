#!/bin/sh

array=(`find -name "*.[h|c|S]"`)

for w in ${array[@]}
do
	echo "$w"
	iconv -c -f utf-8 -t gb2312 $w >> ${w}".gb"
	mv ${w}".gb" ${w} -f
done

