#!/bin/bash
files=`find . -type f -regex '.*\.\(c\|h\|sh\)'`
echo -e "Number of files: "
echo $files | wc -w
echo -e "Number of lines: "
cat $files | wc -l
