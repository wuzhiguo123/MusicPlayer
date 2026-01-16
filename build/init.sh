#!/bin/bash

id=`ipcs | grep 0x6603fd03 | awk {'print $2'}`

if [ ! -z $id ]; then
    ipcrm -m $id
fi
mkfifo  cmd_fifo