#!/bin/bash
set timeout 20

sleep 1
{ echo "write router/igmp.last_member_query_count 10"; echo "quit"; sleep 1; } | telnet localhost 10001
sleep 1
{ echo "write client21/igmp.join 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003
sleep 5
{ echo "write client21/igmp.leave 224.4.4.4"; echo "quit"; sleep 1; } | telnet localhost 10003




