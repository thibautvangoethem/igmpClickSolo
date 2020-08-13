#!/bin/bash
set timeout 20


{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 3
{ echo "write client31/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10005
sleep 3
{ echo "write client32/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10006
sleep 3
{ echo "write client21/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 3
{ echo "write client32/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10006
sleep 5
{ echo "write client22/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10004
sleep 5
{ echo "write client31/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10005
sleep 5
{ echo "write client22/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10004

