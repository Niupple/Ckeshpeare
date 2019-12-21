#!/bin/bash
rm mips.txt
rm myresult.txt
rm -r wrong_samples
mkdir wrong_samples

g++ -g -Wall ./*.cpp 

for file in ./*.in
do
   if [ $file ]; then
     echo "testing $file"
cp $file ./testfile.txt

./a.out 1>/dev/null
java -jar MARS-jdk7-Re.jar mips.txt 1>myresult.txt

if diff -q myresult.txt .txt ;then
	echo "~~~~~~~~~~~~~~~~~~~the Same~~~~~~~~~~~~~~~~~"
	rm testfile.txt
else 
	echo "WAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWAWA"
	cp $file ./wrong_samples/
	new=${file#*/}
	d="cp myresult.txt ./wrong_samples/my_${new%.*}.out"
	eval $d
	s="cp ${new%.*}.out ./wrong_samples/"
	eval $s
fi

fi
done
