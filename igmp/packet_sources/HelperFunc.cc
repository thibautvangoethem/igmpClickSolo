#include <click/config.h>
#include "HelperFunc.hh"

CLICK_DECLS

HelperFunc::HelperFunc() {};

HelperFunc::~HelperFunc() {};

int HelperFunc::deduceIntFromCode(uint8_t  code) {
    int deduced;
    if ((code >> 7) & 1){
        uint8_t exp = (code << 1);
        exp = exp >> 5;
        uint8_t mant = (code << 4);
        mant = mant >> 4;
        deduced = (mant | 0x10 ) << (exp + 3);
    } else {
        deduced = code;
    }
    return deduced;
};

uint8_t HelperFunc::deduceCodeFromInt(uint32_t value){
    
    uint8_t code;


    if (value < 128){
        code = value;
    }else{
        int n = value;
        //most significant bit counted from least significant bit
        int msb = -1; 
        while (n != 0) { 
            n = n / 2; 
            msb++; 

        }
        msb -= 3; //because we shift 3 places
        uint32_t mantAndExpo = value >> 3;
        uint8_t expo = msb - 4;
        uint8_t mant = mantAndExpo >> expo;
        mant = ((mant | 0xf0) xor 0xf0);
        //combine the three parts to get the code
        code = 1;
        code = (code << 3) + expo;
        code =  (code << 4) + mant;
    }

    return code;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(HelperFunc)
