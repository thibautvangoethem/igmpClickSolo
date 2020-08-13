#!/bin/bash
set timeout 20

sleep 1
{ echo "write router/igmp.query_interval 2"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write router/igmp.sendQuery"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write router/igmp.set_igmp_type 2"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 5
{ echo "write client21/igmp.set_igmp_type 3"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 1
{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003




