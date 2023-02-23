#include <stdint.h>
#include <pic32mx.h>

int getsw(void){
    volatile int swValue = (PORTD >> 8) & 0xf;
    return swValue;
}

int getbtns(void){
    volatile int btnValue = (PORTD >> 5) & 0x7;
    return btnValue;
}