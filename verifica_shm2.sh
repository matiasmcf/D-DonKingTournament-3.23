#!/bin/bash
#Verifica proceso monitor

ps -A | grep verifica_shm.sh 1> /dev/null
#echo $? retorna 0 si el proceso existe
VALOR="$?"
if (($VALOR!=0))

then
#inicia monitor en caso de no encontrarse
./verifica_shm.sh


fi

exit 0
