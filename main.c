#include <pic32mx.h>
#include <stdint.h>
#include <display.c>
#include <inputs.c>


struct point {
	int x;
	int y;
};


void user_isr( void ) {

if(IFS(0) & 0x100){
  IFSCLR(0) = 0x100;
  //updateScreen(displaybuffer);
  }
}

int main() {
	display_init();
	init();
	struct point ball;
 	 ball.x = 14;
 	 ball.y = 6;
	 int direction = 1;
	 int yDirection = 1;
	//renderPoint(ball.x,ball.y);
	
	while(1){
		if(getbtns() & BTN1_MASK){
			PORTESET = 0x1;
		}

		if(getbtns() & BTN2_MASK){
			PORTESET = 0x2;
		}

		if(getbtns() & BTN3_MASK){
			PORTESET = 0x4;
		}

		if(getbtns() & BTN4_MASK){
			PORTESET = 0x8;
		}

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
