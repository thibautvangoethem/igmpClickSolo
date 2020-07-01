#ifndef CLICK_VMEMBERSHIP_HH
#define CLICK_VMEMBERSHIP_HH
#include <click/element.hh>
CLICK_DECLS

	/**
	 * helper struct that only denotes an address whihc is used to create and read a packet
	 * */
	struct GrouprecordAddressStruct{
		in_addr address;
	};
	struct GroupRecordStruct{	
			uint8_t recordType;
			uint8_t auxDataLen;
			uint16_t numberSources;
			struct in_addr multicast;

		};
	struct V3MembershipReportPackage {
			uint8_t type;
			uint8_t reserved1;
			uint16_t checksum;
			uint16_t reserved2;
			uint16_t groupRecordNumber;
		};
	class GroupRecord{
		
		private:
			in_addr multicast;
			Vector<in_addr> sources;
			int recordType;
		public:
			GroupRecord(){
				sources=Vector<in_addr>();
			}
			GroupRecord(in_addr mc,Vector<in_addr> srcs,int gr=0){
				multicast=mc;
				sources=srcs;
				recordType=gr;
			}

			in_addr getMulticast();
			Vector<in_addr> getSources();
			int getRecordType();
			
			/**
			 * adds the current data of this GroupRecord  to a packet, this i with igmp ip encap
			 * @param q the packet which the GroupRecord report will be added to
			 * @return the a packet  with the GroupRecord 
			 * */
			GrouprecordAddressStruct* addToPacket(GroupRecordStruct* q);
			/**
			 * helper function that gets the size of this grouprecord in bytes
			 * */
			int getSize();
	};
    
	
	class V3Membership : public Element{
		private:
			Vector<GroupRecord> grouprecords;
			bool checksumValid=true;

		public:
			~V3Membership() {};

			const char *class_name() const	{ return "V3Membership"; }
			const char *port_count() const	{ return "0/0"; }
			const char *processing() const	{ return AGNOSTIC; }
        	int configure(Vector<String>&, ErrorHandler*) {};

			V3Membership(){
				grouprecords=Vector<GroupRecord>();
			}
			V3Membership(const V3Membership&);

			void setGroupRecords(Vector<GroupRecord> records);

			
			/**
			 * function that takes an address and a list of addresses and makes a groupRecord of it which is added to the v3membership vector
			 * */
			void addGroupRecord(in_addr multicast, Vector<in_addr> sources,int recordType=0);

			/**
			 * adds the current data of this v3membership  to a packet, this is with igmp ip encap
			 * @param q the packet which the v3membership report will be added to, should be 0 if you are not adding it on top of anotehr packet
			 * @return the finalised packet with the v3membership 
			 * */
			WritablePacket* addToPacket(WritablePacket* q,in_addr ipsrc,in_addr ipdst,int seq,int igmpType=0x22);		

			/**
			 * static function that reads a packet and return a V3Membership report
			 * This packet must start with an ip header with the option, and mu st be valid
			 * */	
			static V3Membership readPacket(WritablePacket* q);

			Vector<GroupRecord> getRecords();
			bool isChecksumCorrect();

			void setChecksumCorrect(bool correct);
	};

	
CLICK_ENDDECLS
#endif