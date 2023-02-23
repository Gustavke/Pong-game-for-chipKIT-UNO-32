#include <pic32mx.h>
#include <stdint.h>

#define DISPLAY_VDD PORTFbits.RF6
#define DISPLAY_VBATT PORTFbits.RF5
#define DISPLAY_COMMAND_DATA PORTFbits.RF4
#define DISPLAY_RESET PORTGbits.RG9


#define DISPLAY_VDD_PORT PORTF
#define DISPLAY_VDD_MASK 0x40
#define DISPLAY_VBATT_PORT PORTF
#define DISPLAY_VBATT_MASK 0x20
#define DISPLAY_COMMAND_DATA_PORT PORTF
#define DISPLAY_COMMAND_DATA_MASK 0x10
#define DISPLAY_RESET_PORT PORTG
#define DISPLAY_RESET_MASK 0x200


char textbuffer[4][16];

uint8_t displaybuffer[128*4] = {0};

struct point {
	int x;
	int y;
};



void renderPoint(int x, int y){
	int page = y/8;
	displaybuffer[page * 128 + x] = (uint8_t)(1 << y - 8 * page);
}

void delay(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 0x01));
	return SPI2BUF;
}

void display_init() {
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	delay(10);
	DISPLAY_VDD_PORT &= ~DISPLAY_VDD_MASK;
	delay(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_RESET_PORT &= ~DISPLAY_RESET_MASK;
	delay(10);
	DISPLAY_RESET_PORT |= DISPLAY_RESET_MASK;
	delay(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_VBATT_PORT &= ~DISPLAY_VBATT_MASK;
	delay(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}

void updateScreen(const uint8_t *data) {
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x00); //set low nybble of column
		spi_send_recv(0x10); //set high nybble of column

		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 128; j++)
			spi_send_recv(data[i*128 + j]);
	}
}

void clearScreen(uint8_t * buffer)
{	int i;
	for (i = 0; i < 128*4; i++) {
		buffer[i] = 0;}
}

void user_isr( void ) {

if(IFS(0) & 0x100){
  IFSCLR(0) = 0x100;
  //updateScreen(displaybuffer);
  }
}

void init( void )
{
  volatile int* ledControl = (volatile int*) 0xbf886100; //Initialize pointer to TRISE
  *ledControl = *ledControl & ~(0xff); //Set bit 0-7 to 0 (output)
  volatile int* leds = (volatile int*) 0xbf886110; //Initialize pointer to PORTE
  *leds = 0; //Turn of leds
  TRISDSET = 0xfe0; // Set bit 5-11 to input (buttons and switches)
  T2CON = 0x7 << 4; //Disable timer and set prescaling 1:256
  PR2 = 80000000 / 256 / 10; //Period value for 0,1s
  TMR2 = 0; //Resetting counter
  T2CONSET = 1 << 15; //Start timer

  IEC(0) = 0x180; //Enable interrupt for timer2 and int1
  IPC(2) = 4;  //Set timer 2 priority and subpriority 
  IPC(1) = 1 << 26;
  //enable_interrupt(); //Enable interrupts globally 

  
  return;
}

int main() {
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

	display_init();
	init();
	struct point ball;
 	 ball.x = 14;
 	 ball.y = 6;
	 int direction = 1;
	 int yDirection = 1;
	//renderPoint(ball.x,ball.y);
	
	while(1){
		clearScreen(displaybuffer);
		renderPoint(ball.x,ball.y);
		updateScreen(displaybuffer);
		if(ball.x == 127){
			direction = - 1;
		}
		if(ball.x == 0){
			direction = 1;
		}
		if(ball.y == 31){
			yDirection = - 1;
		}
		if(ball.y == 0){
			yDirection = 1;
		}
		ball.x += direction;
		ball.y += yDirection;
		delay(20000);
	};
	return 0;
} 
