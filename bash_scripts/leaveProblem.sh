#!/bin/bash
set timeout 20

sleep 8
{ echo "write client31/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10005
sleep 3
{ echo "write client32/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10006
sleep 3
{ echo "write client32/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10006
sleep 8
{ echo "write client31/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10005


