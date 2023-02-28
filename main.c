#include <stdio.h>
#include <errno.h>
#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>
#include "inputs.h"
#include <math.h>

#define PADDLE_SIZE 9
#define PADDLE_MAX 31-PADDLE_SIZE/2
#define PADDLE_MIN PADDLE_SIZE/2
#define BALL_SIZE 3
#define STATE_MENU 0
#define STATE_SINGLEPLAYER 1
#define STATE_MULTIPLAYER 2
#define STATE_VIEWHIGHSCORE 3

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
uint8_t gameState = STATE_MENU;

uint8_t score_p1;
uint8_t score_p2;

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

	if(ball->x < 1 + BALL_SIZE/2){
		ball->x = 64;
		ball->y = 16;
		ball->xSpeed = - ball->xSpeed;
		ball->ySpeed = - ball->ySpeed;
		score_p2++;
		PORTESET = 1 << (score_p2 - 1);
	}

	if(ball->x > 127 - BALL_SIZE/2){
		ball->x = 64;
		ball->y = 16;
		ball->xSpeed = - ball->xSpeed;
		ball->ySpeed = - ball->ySpeed;
		score_p1++;
		PORTESET = 1 << (7 - (score_p1 - 1));
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

	score_p1 = 0;
	score_p2 = 0;

	//int randomValue = rand() % 8;
	while(1){
		clearScreen();
		if(getbtns() & BTN1_MASK){
			if(paddle2.y < PADDLE_MAX){
			paddle2.y++;
			}
		}

		if(getbtns() & BTN2_MASK){
			if(paddle2.y > PADDLE_MIN){
			paddle2.y--;
			}
		}

		if(getbtns() & BTN3_MASK){
			if(paddle1.y < PADDLE_MAX){
			paddle1.y++;
			}
		}

		if(getbtns() & BTN4_MASK){
			if(paddle1.y > PADDLE_MIN){
			paddle1.y--;
			}
		}

		//paddle2.y = ball.y + randomValue;

		collision(&ball, paddle1, paddle2);
		ball.x += ball.xSpeed;
		ball.y += ball.ySpeed;
		renderPaddle(paddle1);
		renderPaddle(paddle2);
		renderBall(ball);
		updateScreen();
		delay(80000);
	}
}

void menu(void){
	display_string(1, "Singleplayer");
	display_string(2, "Multiplayer");
	display_string(3, "Highscore");

	display_update();
	while(!gameState){
		if(getbtns() & BTN2_MASK){
			gameState = STATE_VIEWHIGHSCORE;
			
		}

		if(getbtns() & BTN3_MASK){
			gameState = STATE_MULTIPLAYER;
			
		}

		if(getbtns() & BTN4_MASK){
			gameState = STATE_SINGLEPLAYER;
			}
	}
}

int main() {
	srand(0x8924152c);
	init();
	display_init();
	while(1){
		switch (gameState)
		{
		case 0:
			menu();
			break;
		case 1:
			gameLoop();
			break;
		case 2: 
			gameLoop();
			break;
		case 3:
			menu();
			break;
		}
	}
	//menu();
	//gameLoop();
	//enable_interrupt(); //Enable interrupts globally 
	return 0;
} 


void user_isr( void ) {
	
	if(IFS(0) & 0x100){
		IFSCLR(0) = 0x100;
		
		
  //updateScreen(displaybuffer);
  }
}