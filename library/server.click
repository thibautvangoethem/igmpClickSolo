// Output configuration:
//
// Packets for the network are put on output 0
// Packets for the host are put on output 1

elementclass Server {
	$address, $gateway |
	arpq :: ARPQuerier($address)

	ip :: Strip(14)
		-> CheckIPHeader()
		-> rt :: StaticIPLookup(
					$address:ip/32 0,
					$address:ipnet 0,
					0.0.0.0/0.0.0.0 $gateway 1)
		-> hostqueue::Queue();
	
	rt[1]
		->igmp_cl :: Classifier(9/02 24/11 ,- )
		-> hostqueue;

	hostqueue
		-> Unqueue()
		-> [1]output;


	igmp_cl[1]
		-> udp_cl::Classifier(9/11,-)
		-> flowqueue::Queue();
	
	udp_cl[1]
		-> DropBroadcasts
		-> flowqueue;

	flowqueue
		-> ipgw :: IPGWOptions($address)
		-> FixIPSrc($address)
		-> Unqueue()	
		-> ttl :: DecIPTTL
		-> frag :: IPFragmenter(1500)
		-> arpq
		-> output;

	ipgw[1] -> ICMPError($address, parameterproblem) -> output;
	ttl[1]  -> ICMPError($address, timeexceeded) -> output;
	frag[1] -> ICMPError($address, unreachable, needfrag) -> output;

	// Incoming Packets
	input
		-> HostEtherFilter($address)
		-> in_cl :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800)
		-> arp_res :: ARPResponder($address)
		-> output;

	in_cl[1] -> [1]arpq;
	in_cl[2] -> ip;
}
