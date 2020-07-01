#ifndef CLICK_MEMBERSHIPQUERY_HH
#define CLICK_MEMBERSHIPQUERY_HH
#include <click/element.hh>
CLICK_DECLS
    
struct MembershipQueryMessage {
    uint8_t type;
    uint8_t max_resp_code;
    uint16_t checksum;
    uint32_t group_address;
	//this order is different of the protocol because of the way gcc compiler does byte alignment
    unsigned int qrv : 3;
    unsigned int s : 1;
    unsigned int resv : 4;
    uint8_t qqic;
    uint16_t numberOfSources;
};

class MembershipQuery: public Element {
	private:
		uint8_t queryType;
		int maxResp;
		uint32_t groupAdd;
		bool sFlag;
		int QRV;
		int QQIC;
		Vector<uint32_t> sources;

		//These 2 variables will be the read in ip header source and destination
		in_addr readIpsrc;
		in_addr readIpdst;


	public:
		~MembershipQuery () {};

		const char *class_name() const	{ return "MembershipQuery"; }
		const char *port_count() const	{ return "0/0"; }
		const char *processing() const	{ return AGNOSTIC; }
        int configure(Vector<String>&, ErrorHandler*) {return 0;};

		MembershipQuery(){
			sources=Vector<uint32_t>();
		}

		void setType(int type);
		void setMaxResp(int max);
		int getMaxResp();
		void setGroupAddr(uint32_t group);
		void setSFlag(bool flag);
		void setQRV(int qrv);
		int getQRV();
		void setQQIC(int qqic);
		int getQQIC();
		void addSource(uint32_t source);
		void setReadIpSrc(in_addr src);
		in_addr getReadIpSrc();
		void setReadIpDst(in_addr dst);
		in_addr getReadIpDst();


		/**
		 * a copy constructor 
		 * */
		MembershipQuery(const MembershipQuery &p2) {
			queryType=p2.queryType;
			maxResp=p2.maxResp;
			groupAdd=p2.groupAdd;
			sFlag=p2.sFlag;
			QRV=p2.QRV;
			QQIC=p2.QQIC;
			sources=p2.sources;
			readIpdst=p2.readIpdst;
			readIpsrc=p2.readIpsrc;
		 } 


		/**
		 * adds the current data of this MembershipQuery  to a packet, this is with igmp ip encap
		 * @param q the packet which the MembershipQuery will be added to, should be 0 if you are not adding it on top of anotehr packet
		 * @return the finalised packet with the MembershipQuery 
		 * */
		WritablePacket* addToPacket(WritablePacket* q,in_addr ipsrc,in_addr ipdst,int seq, bool correctChecksum);		

		/**
		 * static function that reads a packet and return a MembershipQuery
		 * This packet must start with an ip header with the option, and must be valid
		 * */	
		static MembershipQuery readPacket(WritablePacket* q);

		
		
};
CLICK_ENDDECLS
#endif