#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include "nullAgnost.hh"

CLICK_DECLS
NullAgnost::NullAgnost()
{}

NullAgnost::~ NullAgnost()
{}

int NullAgnost::configure(Vector<String> &conf, ErrorHandler *errh) {
	return 0;
}

Packet* NullAgnost::pull(int){
	Packet* p = input(0).pull();
	if(p == 0){
		return 0;
	}
	return p;
}

void NullAgnost::push(int, Packet *p){
     output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(NullAgnost)
