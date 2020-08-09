#!/bin/bash
set timeout 20
{ echo "write router/igmp.robustness 5"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write router/igmp.sendQuery"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write client21/igmp.unsolicited_report_interval 5"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 1
{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003



