#!/bin/bash
set timeout 20

sleep 1
{ echo "write client21/igmp.invalid_igmp_checksum true"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 1
{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 4
{ echo "write client21/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 4
{ echo "write client21/igmp.invalid_igmp_checksum false"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 3
{ echo "write router/igmp.invalid_igmp_checksum true"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write router/igmp.robustness 4"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write router/igmp.sendQuery"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 5
{ echo "write client21/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003





