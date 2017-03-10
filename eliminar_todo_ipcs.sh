#!/bin/bash

pkill -9 donkey
pkill -9 serverPartida
pkill -9 serverTorneo
USUARIO=`whoami`

IPCS_S=`ipcs -s | egrep "0x[0-9a-f]+ [0-9]+" | grep $USUARIO | cut -f2 -d" "`
IPCS_M=`ipcs -m | egrep "0x[0-9a-f]+ [0-9]+" | grep $USUARIO | cut -f2 -d" "`
IPCS_Q=`ipcs -q | egrep "0x[0-9a-f]+ [0-9]+" | grep $USUARIO | cut -f2 -d" "`


for id in $IPCS_M; do
  ipcrm -m $id 2> /dev/null
done

for id in $IPCS_S; do
  ipcrm -s $id 2> /dev/null
done

for id in $IPCS_Q; do
  ipcrm -q $id 2> /dev/null
done

rm ./.PID/* 2> /dev/null
rm /dev/shm/sem.mtx* 2> /dev/null

exit 0
