#!/bin/sh
sudo cangw -A -s vcan0 -d can0 -e -X
sudo cangw -A -s can0 -d vcan0 -e -X