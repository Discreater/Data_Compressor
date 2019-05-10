#! /bin/bash



loopnum=0

if [ $# == 0 ]
then
	loopnum=5
else
	loopnum=$1
fi

while(( $loopnum>0 ))
do
	echo $loopnum
	let "loopnum--"
done

echo "Hello World!"
exit 0

