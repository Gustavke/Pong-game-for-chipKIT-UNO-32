#include <stdint.h>
#include <pic32mx.h>

#define BTN1_MASK 0x1
#define BTN2_MASK 0x2
#define BTN3_MASK 0x4
#define BTN4_MASK 0x8

/*
int getsw(void){
    volatile int swValue = (PORTD >> 8) & 0xf;
    return swValue;
}
*/

int getbtns(void){
    volatile int btnValue = ((PORTD >> 4) & 0xe) | ((PORTF >> 1) & 0x1);
    return btnValue;
}