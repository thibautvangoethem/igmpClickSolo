#!/bin/bash
set timeout 20

sleep 10
{ echo "write router/igmp.query_interval 2"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1

