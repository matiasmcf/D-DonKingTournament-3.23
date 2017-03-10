#!/bin/bash

USUARIO=`whoami`

IPCS_M=` ipcs -m | egrep "0x[1-9a-f]+[0-9]+" | grep -c "660"`
SEM=`ls /dev/shm | egrep -c "sem.mtx"`
sleep 2

X=1


while [ $X -eq 1 ]
do
	IPCS_M2=` ipcs -m | egrep "0x[1-9a-f]+[0-9]+" | grep -c "660"`
	SEM2=`ls /dev/shm | egrep -c "sem.mtx"`
	if [ $IPCS_M -ne $IPCS_M2 ]
	then
		X=0
		echo "MONITOR: Se elimino memoria compartida."
	fi
	if [ $SEM -ne $SEM2 ]
	then
		X=0
		echo "MONITOR: Se elimino semaforo nombrado."
	fi
	sleep 2
done

./eliminar_todo_ipcs.sh

exit 0


