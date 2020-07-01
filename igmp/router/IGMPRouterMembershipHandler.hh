#ifndef CLICK_IGMPROUTERMEMBERSHIPHANDLER_HH
#define CLICK_IGMPROUTERMEMBERSHIPHANDLER_HH
#include <click/element.hh>
#include <click/timer.hh>

CLICK_DECLS
//forward declaration for V3Membership because click compiler keeps throwing errors
class V3Membership;
class HelperFunc;

/**
 * copied and modified from click udp as it isnt a complete type
 * */
struct igmp_udp {
    uint16_t	uh_sport;		/* 0-1   source port		 */
    uint16_t	uh_dport;		/* 2-3   destination port	 */
    uint16_t	uh_ulen;		/* 4-5   UDP length		     */
    uint16_t	uh_sum;			/* 6-7   checksum		     */
};

class IGMPRouterMembershipHandler;

struct InterfaceReceptionState: public Element{
	//initialisation as a class.
	InterfaceReceptionState() {};
	InterfaceReceptionState(IGMPRouterMembershipHandler* router);
	~InterfaceReceptionState() {};

	const char *class_name() const	{ return "InterfaceReceptionState"; }
	const char *port_count() const	{ return "0/0"; }
	const char *processing() const	{ return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	//method to handle the running of timers.
	void run_timer(Timer*);

	//this member is kept so we are able to know on which interface the record is kept.
	int interface;
	
	//we keep the router on which the interface exist in order to call for the removal of this interface
	IGMPRouterMembershipHandler* owningRouter;

	//from here onwards this is actually the conceptual record.
	in_addr multicast;
	Timer *groupTimer;
	//true is include, false is exclude
	//will always be exclude if the interfaceReceptionstate exist, join has always exclude mode in this implementation.
	//Rfc states if a group record with a filter-mode of EXCLUDE is received, the router filter-mode for that
	//group will be EXCLUDE.
	bool filterMode;
	//this list is always empty because no join or leave is specific on sources.
	Vector<in_addr> sourceList;
};

struct startUpQueryTimerData{
	IGMPRouterMembershipHandler* me;
	int count;
};

//struct neceassary for timers
struct lastMemberQueryTimerData{
	IGMPRouterMembershipHandler* me;
	int count;
	in_addr address;
	InterfaceReceptionState* receptionState;
	uint32_t srcInt;
};

class IGMPRouterMembershipHandler : public Element { 
	public:
		IGMPRouterMembershipHandler();
		~IGMPRouterMembershipHandler();
		
		const char *class_name() const	{ return "IGMPRouterMembershipHandler"; }
		const char *port_count() const	{ return "-/-"; }
		const char *processing() const	{ return PUSH; }
		int configure(Vector<String>&, ErrorHandler*);

		static int sendQuery(const String &conf, Element *e,void * thunk, ErrorHandler * errh);
		//the following handlers are special handlers for testing purposes
		static int setRobustness(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setQueryInterval(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setMaxResponseTime(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setLastMemberQueryInterval(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setLastMemberQueryCount(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setIGMPType(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setIGMPAddress(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		static int setIGMPCheckSum(const String &conf, Element *e, void *thunk, ErrorHandler *errh);
		void add_handlers();

		in_addr getdst();
		in_addr getn1();
		in_addr getn2();
		in_addr getserver();
		int getseq();
		void incrseq();

		void removeInterface(InterfaceReceptionState* interface);

		//needs to be pointer otherwise forward declaration wont work
		void handleMembershipReport(in_addr src,int interface,V3Membership* mem);

		void sendGeneralQuery();
		void handleUDPPacket(int interface,Packet* p);
		
		void push(int, Packet *);
		void sendQuery(int, Packet *);

		MembershipQuery makeGeneralQuery();
		MembershipQuery makeGroupSpecificQuery(uint32_t group);

		void run_timer(Timer*);

		static void handleExpiryLastMemberQueryTimer(Timer*, void *);

		static void handleExpirySQT(Timer*, void *);

		void expireLMQT(InterfaceReceptionState*);
		
	private:

		//members to send memebrship query
		in_addr src;
		IPAddress _dstIP;
		IPAddress network1;
		IPAddress network2;
		IPAddress server;
		int _sequence;

		Timer queryIntervalTimer;

		//we keep a vector to know if there is already a lmqtData struct on a certain interfaceReceptionState.
		Vector<lastMemberQueryTimerData*> lmqtDataVec;

		//member to keep timer values
		int robustnessVariable;
		int querierInterval;
		//startupQueryInterval is calculated from querierInterval
		int startupQueryCount;
		int queryResponseInterval; //in deciseconds
		//the groupMembershipInterval is calculated from other values.
		int lastMemberQueryInterval; //in deciseconds
		int lastMemberQueryCount;
		int typeQueries;
		in_addr multicastAddress;
		bool checksumValid;

		//members to handle membership reports, per interface a vector for each multicast (thus joining,leaving and forwarding)
		Vector<InterfaceReceptionState*> interfaces;
};

CLICK_ENDDECLS
#endif
