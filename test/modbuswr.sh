#! /bin/bash

if [ -z "$1" ] || [ -z "$2" ]
then
	echo usage: $0 \<address\> \<value\> \<count-words=1\>
	exit 0
fi

if [ -z "$3" ]
then
	bytes=2
else
	bytes=$(($3 * 2))
fi

value_int=$2

for i in $(seq 1 $bytes)
do
	value_x=$(printf "\\\x%02x%s" $(($value_int % 256)) "$value_x")
	value_int=$(($value_int / 256))
done

if [ $value_int -ne 0 ]
then
	echo "value too large"
	exit 1
fi

address_x=$(printf "\\\x%02x\\\x%02x" $(($1 / 256)) $(($1 % 256)))
if [ $bytes -eq 2 ]
then
	length_x=$(printf "\\\x%02x\\\x%02x" $((($bytes + 4) / 256)) $((($bytes + 4) % 256)))
	transmit="\x12\x34\x00\x00${length_x}\x01\x06${address_x}${value_x}"
else
	count_x=$(printf "\\\x%02x\\\x%02x" $(($3 / 256)) $(($3 % 256)))
	length_x=$(printf "\\\x%02x\\\x%02x" $((($bytes + 7) / 256)) $((($bytes + 7) % 256)))
	bytes_x=$(printf "\\\x%02x" $bytes)
	transmit="\x12\x34\x00\x00${length_x}\x01\x10${address_x}${count_x}${bytes_x}${value_x}"
fi

echo "          00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f"
echo "---------+-----------------------------------------------"
echo -n -e "$transmit" | xxd -g 1 | sed 's/[0-9a-f]*:/transmit:/1' | sed 's/  .*$//g'
echo -n -e "$transmit" | nc -N 192.168.56.101 502 | xxd -g 1 | sed 's/[0-9a-f]*:/ receive:/1' | sed 's/  .*$//g'
