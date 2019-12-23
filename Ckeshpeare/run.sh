#!/bin/bash
for i in 4U/*.in
do
	echo testcase \#$i
	cp $i testfile.txt && ./test.exe >/dev/null 2>/dev/null && java -jar mars.jar mips.txt > output
	new=${i#*/}
	diff output 4U/${new%.*}.out
	if [ $? -eq 0 ]; then
		echo 'OK!'
	else
		echo 'WA!'
		exit 1
	fi
done
