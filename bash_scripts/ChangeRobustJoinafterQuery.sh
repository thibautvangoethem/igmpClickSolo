#!/bin/bash
set timeout 20


#!/bin/bash
set timeout 20

{ echo "write router/igmp.robustness 4"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 10s
{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 5
{ echo "write client21/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003

