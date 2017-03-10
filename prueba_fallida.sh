#!/bin/bash
pkill -9 monitor
pkill -9 serverPartida
pkill -9 donkey
pkill -9 serverTorneo
rm ./.PID/*.txt 2> /dev/null
./eliminar_todo_ipcs.sh
exit 0
