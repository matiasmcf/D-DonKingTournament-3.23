#!/bin/bash
#Verifica proceso monitor

ps -A | grep monitor 1> /dev/null
#echo $? retorna 0 si el proceso existe
VALOR="$?"
if (($VALOR!=0))

then
#inicia monitor en caso de no encontrarse
./monitor


fi

exit 0
