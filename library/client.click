// Output configuration: 
//
// Packets for the network are put on output 0
// Packets for the host are put on output 1

elementclass Client {
	$address, $gateway |
	arpq :: ARPQuerier($address)
	igmp::ClientInterface($address, 224.0.0.22,224.0.0.1)
	

	ip :: Strip(14)
		-> CheckIPHeader()
		-> rt :: StaticIPLookup(
					$address:ip/32 0,
					$address:ipnet 0,
					0.0.0.0/0.0.0.0 $gateway 1)		
		-> [1]output;
	

	rt[1]
		->igmp_cl :: Classifier(9/02 24/11 ,- )
		->igmp
		->arpq
		->output;

	igmp_cl[1]
		-> udp_cl::Classifier(9/11,-)
		->[1]igmp[1]
		->flowqueue::Queue()

	udp_cl[1]->flowqueue;

	flowqueue
		-> ipgw :: IPGWOptions($address)
		-> FixIPSrc($address)
		->Unqueue()
		-> ttl :: DecIPTTL
		-> frag :: IPFragmenter(1500)
		-> [1]output;

	ipgw[1] -> ICMPError($address, parameterproblem) -> output;
	ttl[1]  -> ICMPError($address, timeexceeded) -> output;
	frag[1] -> ICMPError($address, unreachable, needfrag) -> output;
	
	// Incoming Packets
	input
		-> HostEtherFilter($address)
		-> in_cl :: Classifier(12/0806 20/0001, 12/0806 20/0002, 12/0800)
		-> arp_res :: ARPResponder($address)
		-> arpq
		-> output;

	in_cl[1] -> [1]arpq[1] -> [1]output;
	in_cl[2] -> ip;
}
