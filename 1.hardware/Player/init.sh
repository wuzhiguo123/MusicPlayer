#!/bin/bash

shmid=`ipcs | grep 0x662301cd | awk {'print $2'}`

if [ ! -z $shmid ]; then
	ipcrm -m $shmid
fi

semid=`ipcs | grep 0x000003e8 | awk {'print $2'}`

if [ ! -z $semid ]; then
	ipcrm -s $semid
fi

rm -rf /root/fifo

mkdir /root/fifo

mkfifo /root/fifo/cmd_fifo
mkfifo /root/fifo/asr_fifo

