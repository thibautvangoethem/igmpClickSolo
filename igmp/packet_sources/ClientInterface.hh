#ifndef CLICK_INTERNALCLIENT_HH
#define CLICK_INTERNALCLIENT_HH
#include <click/element.hh>
#include <click/vector.hh>

CLICK_DECLS

enum recordType {MODE_IS_INCLUDE=1, MODE_IS_EXCLUDE, CHANGE_TO_INCLUDE_MODE, CHANGE_TO_EXCLUDE_MODE};
class ClientInterface : public Element {
	recordType mode;
	in_addr currentJoinedGroup;
    IPAddress _srcIP;
    IPAddress _maIP;
    IPAddress igmpBroadcast;
    int _sequence;
    int robustness=2;

    //all variables here are used to test the igmp implementation, if on 0 they do not matter
    int robustnessOverride=0;
    //in seconds
    int uri=1;
    int igmpType=0;
    //if anything other than 0 then you have to change the group type
    int groupRecordType=0;
    bool invalidChecksum=false;

    //timerStructs
    struct groupRecordTimerStruct{ // callback data
        ClientInterface* me;
        Packet* membershipPacket;
        int sendsLeft;
    };

    public:
        ClientInterface();
        ~ClientInterface();

        const char *class_name() const	{ return "ClientInterface"; }
        /**
         * Port 0 is for igmp messages and port 1 is for udp packages
         * */
		const char *port_count() const	{ return "2/2"; }
		const char *processing() const	{ return PUSH; }
        int configure(Vector<String>&, ErrorHandler*);

        void push(int, Packet*);

        /**
         * This function makes a grouprecord packet that contains its information
         * This packet should be send as a response for a membership query
         * @param q This pointer will be used to construct the packet, if not needed just give a nullpointer
         * */
        Packet* makeGroupRecordPacket(WritablePacket *q);

        /**
         * This is placed in a seperate function so we can correctly start up the timers
         * */
        void sendRobustMembershipPacket(Packet *q,int left,double maxresp);

        void add_handlers();

        /**
         * join handler that sends a join query to the router
         * */
        static int join(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        /**
         * leave handler that sends a leave query to the router
         * */
	    static int leave(const String &conf, Element *e, void* thunk, ErrorHandler * err);

        static int robustnessHandler(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        static int unriHandler(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        static int igmpTypeHandler(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        static int igmpAddressHandler(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        static int groupRecordTypeHandler(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        static int checksumHandler(const String &conf, Element *e, void* thunk, ErrorHandler * errh);

        void setRobustness(int newValue);
        int getRobustness();
        void setUnsolicitedReportInterval(int newValue);
        void setIgmpType(int newValue);
        void setIgmpAddress(in_addr newValue);
        void setGroupRecordType(int newValue);
        void setInvalidChecksum(bool newValue);

        static void handleExpiry(Timer*, void *);

        void expire(Packet*,int sendsLeft);

    
};

CLICK_ENDDECLS
#endif

