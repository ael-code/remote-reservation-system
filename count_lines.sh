#!/bin/bash
files=`find . -type f -regex '.*\.\(c\|h\|sh\)'`
cat $files | wc -l
