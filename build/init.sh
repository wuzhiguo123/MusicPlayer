#!/bin/bash

shmid=`ipcs | grep 0x6603fd03 | awk {'print $2'}`

if [ ! -z $shmid ]; then
    ipcrm -m $shmid
fi

semid=`ipcs | grep 0x000003e8 | awk {'print $2'}`

if [ ! -z $semid ]; then
    ipcrm -s $semid
if
mkfifo cmd_fifo