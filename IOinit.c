#include <pic32mx.h>
#include <stdint.h>

void *stdin, *stdout, *stderr, *errno;

void init( void )
{
    /* Set up peripheral bus clock */
	OSCCON &= ~0x180000;
	OSCCON |= 0x080000;
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	
	/* Clear SPIROV*/
	SPI2STATCLR &= ~0x40;
	/* Set CKP = 1, MSTEN = 1; */
        SPI2CON |= 0x60;
	
	/* Turn on SPI */
	SPI2CONSET = 0x8000;


    volatile int* ledControl = (volatile int*) 0xbf886100; //Initialize pointer to TRISE
    *ledControl = *ledControl & ~(0xff); //Set bit 0-7 to 0 (output)
    volatile int* leds = (volatile int*) 0xbf886110; //Initialize pointer to PORTE
    *leds = 0; //Turn of leds
    TRISDSET = 0xfe0; // Set bit 5-11 to input (buttons and switches)
    T2CON = 0x7 << 4; //Disable timer and set prescaling 1:256
    PR2 = 80000000 / 256 / 80; //Period value for 0,1s
    TMR2 = 0; //Resetting counter
    T2CONSET = 1 << 15; //Start timer

    IEC(0) = 0x100; //Enable interrupt for timer2 
    IPC(2) = 4;  //Set timer 2 priority and subpriority 
    IPC(1) = 1 << 26;
    

    
    return;
}