#ifndef CLICK_HELPERFUNC_HH
#define CLICK_HELPERFUNC_HH
#include <click/element.hh>
CLICK_DECLS

/**
 * This class only holds some helper functions, the reason it is in a click element is because without it click doesn't copy it for compiling.
 * */
class HelperFunc: public Element {

    public:
        HelperFunc();
        ~HelperFunc();

        const char *class_name() const	{ return "HelperFunc"; }
		const char *port_count() const	{ return "0/0"; }
		const char *processing() const	{ return AGNOSTIC; }
		int configure(Vector<String>&, ErrorHandler*) {};

        /**
         * Function that turns an int into a float if it's above 128
         * */
        static int deduceIntFromCode(uint8_t code);

        /**
         * Function that return a code based on an int, if int is not representable 
         * we take the maximum representable number lower than the given int
         * */
        static uint8_t deduceCodeFromInt(uint32_t value);

};

CLICK_ENDDECLS
#endif