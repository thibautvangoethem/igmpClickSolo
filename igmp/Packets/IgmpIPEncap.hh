#ifndef CLICK_IGMPIPENCAP_HH
#define CLICK_IGMPIPENCAP_HH
#include <click/element.hh>
CLICK_DECLS

	struct routerAlert{
		uint8_t part1;
		uint8_t part2;
		uint8_t octet1;
		uint8_t octet2;
	};
	
class IgmpIpEncap : public Element {
	private:
	//some helper variables used to get the src and dst ip after reading in the ip header
		in_addr readSrc;
		in_addr readDst;
	public:
		IgmpIpEncap() {};
		~IgmpIpEncap() {};

		const char *class_name() const	{ return "IgmpIpEncap"; }
		const char *port_count() const	{ return "0/0"; }
		const char *processing() const	{ return AGNOSTIC; }
		int configure(Vector<String>&, ErrorHandler*) {};

		/**
		* adds ip encapsulation to the start of a writablepacket
		* @param q the writable packet that has just been made and has the whole data set to 0
		* @param src src ip
		* @param dst dst ip
		* @param seq the sequence number of this packet 
		* @return
		**/ 
		routerAlert* addIP(WritablePacket* q,in_addr src,in_addr dst,uint32_t seq);

		/**
		 * this function reads the igmp ip and discard most of it, but it checks the checksum and will return null if the checksum is incorrect
		 **/
		routerAlert* readIgmpIP(WritablePacket* q);

		/**
		 * helper function that returns the size of the igmp ip header in bytes
		 * @return an int
		 * */ 
		int getIgmpIPSize();

		in_addr getReadSrc();
		in_addr getReadDst();
};
CLICK_ENDDECLS
#endif