#!/bin/bash
FILE=~/.local/lib/wireshark/plugins

[ ! -d "$FILE" ] &&  mkdir -p ${FILE} 


cp ./ics.lua ${FILE}/ics.lua
