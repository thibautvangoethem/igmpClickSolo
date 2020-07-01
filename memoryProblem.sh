#!/bin/bash
set timeout 20


{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 3
{ echo "write client22/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10004
sleep 5
{ echo "write client22/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10004

