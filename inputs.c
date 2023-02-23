#include <stdint.h>
#include <pic32mx.h>

int getsw(void){
    volatile int swValue = (PORTD >> 8) & 0xf;
    return swValue;
}

int getbtns(void){
    volatile int btnValue = ((PORTD >> 4) & 0xe) | ((PORTF >> 1) & 0x1);
    return btnValue;
}