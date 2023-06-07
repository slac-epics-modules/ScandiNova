#! /bin/bash

# This file is part of ScandiNova MKS control system application. It is
# subject to the license terms in the LICENSE.txt file found in the top-level
# directory of this distribution and at
# https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. No part of
# the ScandiNova MKS control system application, including this file, may be
# copied, modified, propagated, or distributed except according to the terms
# contained in the LICENSE.txt file.

if [ -z "$1" ]
then
	echo usage: $0 \<address\> \<count-words=1\>
	exit 0
fi

if [ -z "$2" ]
then
	count_x="\\x00\\x01"
else
	count_x=$(printf "\\\x%02x\\\x%02x" $(($2 / 256)) $(($2 % 256)))
fi

address_x=$(printf "\\\x%02x\\\x%02x" $(($1 / 256)) $(($1 % 256)))
transmit="\x12\x34\x00\x00\x00\x06\x01\x03${address_x}${count_x}"

echo "          00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f"
echo "---------+-----------------------------------------------"
echo -n -e "$transmit" | xxd -g 1 | sed 's/[0-9a-f]*:/transmit:/1' | sed 's/  .*$//g'
echo -n -e "$transmit" | nc -N 192.168.56.101 502 | xxd -g 1 | sed 's/[0-9a-f]*:/ receive:/1' | sed 's/  .*$//g'
