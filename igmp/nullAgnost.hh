#ifndef CLICK_NULLAGNOST_HH
#define CLICK_NULLAGNOST_HH
#include <click/element.hh>
CLICK_DECLS

class NullAgnost : public Element { 
	public:
		NullAgnost();
		~NullAgnost();
		
		const char *class_name() const	{ return "NullAgnost"; }
		const char *port_count() const	{ return "1/1"; }
		const char *processing() const	{ return AGNOSTIC; }
		int configure(Vector<String>&, ErrorHandler*);
		
		Packet* pull(int);
		void push(int, Packet *);
	private:
		uint32_t maxSize;
};

CLICK_ENDDECLS
#endif
