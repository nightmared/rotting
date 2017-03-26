#!/bin/sh -e

cd $(dirname $0)
export LD_LIBRARY_PATH="libs:$LD_LIBRARY_PATH"
export PATH="bin:$PATH"
mosquitto -c mosquitto.conf -v &
python3 run-server.py
