#include <pic32mx.h>
#include <stdint.h>
#include "inputs.h"

#define PADDLE_SIZE 9
#define PADDLE_MAX 31-PADDLE_SIZE/2
#define PADDLE_MIN PADDLE_SIZE/2
#define BALL_SIZE 3
struct point {
	int x;
	int y;
	float xSpeed;
	float ySpeed;
};

struct paddle{
	uint8_t x;
	uint8_t y;
};

void user_isr( void ) {

if(IFS(0) & 0x100){
  IFSCLR(0) = 0x100;
  //updateScreen(displaybuffer);
  }
}

int main() {
	init();
	display_init();
	
	struct point ball;
 	 ball.x = 14;
 	 ball.y = 6;
	 int direction = 1;
	 int yDirection = 0;
	
	struct paddle paddle1;
	paddle1.x = 10;
	paddle1.y = 16;

	struct paddle paddle2;
	paddle2.x = 125;
	paddle2.y = 16;
	
	while(1){
		if(getbtns() & BTN1_MASK){
			PORTESET = 0x1;
		}

		if(getbtns() & BTN2_MASK){
			PORTESET = 0x2;
		}

		if(getbtns() & BTN3_MASK){
			PORTESET = 0x4;
			if(paddle1.y > PADDLE_MIN){
			paddle1.y--;
			}
		}

		if(getbtns() & BTN4_MASK){
			PORTESET = 0x8;
			if(paddle1.y < PADDLE_MAX){
			paddle1.y++;
			}
		}

		clearScreen();
		int i, j;
		for(i = paddle1.y - PADDLE_SIZE/2; i <= paddle1.y + PADDLE_SIZE/2; i++){
			renderPoint(paddle1.x, i);
		}
		for(i = ball.x - BALL_SIZE/2; i <= ball.x + BALL_SIZE/2; i++){
			
			for(j = ball.y - BALL_SIZE/2; j <= ball.y + BALL_SIZE/2; j++){
				renderPoint(i,j);
			}
		}
		updateScreen();
		if(ball.x == 127 - BALL_SIZE/2){
			direction = - 1;
		}
		if(ball.x == paddle1.x + 1 + BALL_SIZE/2 && ball.y - BALL_SIZE/2  <= paddle1.y + PADDLE_SIZE/2 && ball.y + BALL_SIZE/2 >= paddle1.y - PADDLE_SIZE/2){
			direction = 1;
		}
		if(ball.y == 31){
			yDirection = - 1;
		}
		if(ball.y == 0){
			yDirection = 1;
		}

		if(ball.x < 1 + BALL_SIZE/2){
			ball.x = 64;
			ball.y = 16;
		}

		ball.x += direction;
		ball.y += yDirection;
		delay(80000);
	};
	return 0;
} 
