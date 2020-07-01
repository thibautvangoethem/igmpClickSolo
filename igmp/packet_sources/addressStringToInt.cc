#include <click/config.h>
#include "addressStringToInt.hh"

in_addr addressStringToInt(String address)
{

    uint32_t addressInt = 0;
    uint8_t byteNumber = 0;

    for(char& c : address)
    {
        switch (c) 
        {
            case '.': 
                addressInt = (addressInt << 8);
                addressInt = (addressInt || byteNumber);
                byteNumber = 0;
                break;
            case '0': 
                byteNumber *= 10;
                break;
            case '1': 
                byteNumber *= 10;
                byteNumber += 1;
                break;
            case '2': 
                byteNumber *= 10;
                byteNumber += 2;
                break;
            case '3': 
                byteNumber *= 10;
                byteNumber += 3;
                break;
            case '4': 
                byteNumber *= 10;
                byteNumber += 4;
                break;
            case '5': 
                byteNumber *= 10;
                byteNumber += 5;
                break;
            case '6': 
                byteNumber *= 10;
                byteNumber += 6;
                break;
            case '7': 
                byteNumber *= 10;
                byteNumber += 7;
                break;
            case '8': 
                byteNumber *= 10;
                byteNumber += 8;
                break;
            case '9': 
                byteNumber *= 10;
                byteNumber += 9;
                break;
        }
    }
    in_addr result;
    result.in_addr_t = addressInt;	
   
    return result;
}
