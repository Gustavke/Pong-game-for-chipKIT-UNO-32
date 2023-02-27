#include <pic32mx.h>
#include <stdint.h>
#include "inputs.h"
#include <math.h>

#define PADDLE_SIZE 9
#define PADDLE_MAX 31-PADDLE_SIZE/2
#define PADDLE_MIN PADDLE_SIZE/2
#define BALL_SIZE 3
struct point {
	float x;
	float y;
	float xSpeed;
	float ySpeed;
};

struct paddle{
	uint8_t x;
	uint8_t y;
};
/* struct point ball;
struct paddle paddle1;
struct paddle paddle2; */

double sqroot(double square)
{
    double root=square/3;
    int i;
    if (square <= 0) return 0;
    for (i=0; i<32; i++)
        root = (root + square / root) / 2;
    return root;
}


void collision(struct point* ball, struct paddle paddle1, struct paddle paddle2){
	if((int)ball->x == paddle1.x + 1 + BALL_SIZE/2 && ball->y - BALL_SIZE/2  <= paddle1.y + PADDLE_SIZE/2 && ball->y + BALL_SIZE/2 >= paddle1.y - PADDLE_SIZE/2){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
		if(ball->ySpeed > 0.8){
			ball->ySpeed = 0.8;
		}
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
	}
	if((int)ball->x == paddle2.x - BALL_SIZE/2 && ball->y - BALL_SIZE/2  <= paddle2.y + PADDLE_SIZE/2 && ball->y + BALL_SIZE/2 >= paddle2.y - PADDLE_SIZE/2){
			ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
			if(ball->ySpeed > 0.8){
				ball->ySpeed = 0.8;
			}
			ball->xSpeed = - sqroot(1 - ball->ySpeed * ball->ySpeed);
		}

	if(ball->y >= 31 - BALL_SIZE/2){
		ball->ySpeed = - ball->ySpeed;
	}
	if(ball->y <= 0 + BALL_SIZE/2){
		ball->ySpeed = - ball->ySpeed;
	}

	if(ball->x < 1 + BALL_SIZE/2 || ball->x > 127 -BALL_SIZE/2){
		ball->x = 64;
		ball->y = 16;
	}
}

void renderPaddle(struct paddle paddle){
	int i;
	for(i = paddle.y - PADDLE_SIZE/2; i <= paddle.y + PADDLE_SIZE/2; i++){
			renderPoint(paddle.x, i);
		}
}

void renderBall(struct point ball){
	int i, j;
	for(i = ball.x - BALL_SIZE/2; i <= ball.x + BALL_SIZE/2; i++){			
			for(j = ball.y - BALL_SIZE/2; j <= ball.y + BALL_SIZE/2; j++){
				renderPoint(i,j);
			}
		}
}

void gameLoop(){
	struct point ball;
	ball.x = 14;
	ball.y = 6;
	ball.xSpeed = 1/sqroot(2);
	ball.ySpeed = 1/sqroot(2);
	
	struct paddle paddle1;
	paddle1.x = 5;
	paddle1.y = 16;

	struct paddle paddle2;
	paddle2.x = 125;
	paddle2.y = 16;

	while(1){
		clearScreen();
		if(getbtns() & BTN1_MASK){
			PORTESET = 0x1;
			if(paddle2.y > PADDLE_MIN){
			paddle2.y++;
			}
		}

		if(getbtns() & BTN2_MASK){
			PORTESET = 0x2;
			if(paddle2.y < PADDLE_MAX){
			paddle2.y--;
			}
		}

		if(getbtns() & BTN3_MASK){
			PORTESET = 0x4;
			if(paddle1.y > PADDLE_MIN){
			paddle1.y++;
			}
		}

		if(getbtns() & BTN4_MASK){
			PORTESET = 0x8;
			if(paddle1.y < PADDLE_MAX){
			paddle1.y--;
			}
		}

		collision(&ball, paddle1);
		ball.x += ball.xSpeed;
		ball.y += ball.ySpeed;
		renderPaddle(paddle1);
		renderPaddle(paddle2);
		renderBall(ball);
		updateScreen();
		delay(80000);
	}
}

int main() {
	init();
	display_init();
	gameLoop();
	//enable_interrupt(); //Enable interrupts globally 
	return 0;
} 


void user_isr( void ) {
	
	if(IFS(0) & 0x100){
		IFSCLR(0) = 0x100;
		
		
  //updateScreen(displaybuffer);
  }
}