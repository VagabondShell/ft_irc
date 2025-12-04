#!/bin/bash

SERVER_IP="127.0.0.1"
SERVER_PORT="5555" 
DEFAULT_PASS="1"
if [ -z "$1" ]; then
    echo "Usage: $0 <nickname>"
    exit 1
fi

NICKNAME="$1"

{
    echo "PASS ${DEFAULT_PASS}"
    echo "USER USER USER User USER"
    echo "NICK ${NICKNAME}"
    cat
} | nc ${SERVER_IP} ${SERVER_PORT}
