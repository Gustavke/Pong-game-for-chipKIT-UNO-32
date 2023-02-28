#include <stdio.h>
#include <errno.h>
#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>
#include "inputs.h"
#include <math.h>
#include <string.h>

#define PADDLE_HEIGHT 9
#define PADDLE_WIDTH 3
#define PADDLE_MAX 31-PADDLE_HEIGHT/2
#define PADDLE_MIN PADDLE_HEIGHT/2
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
int highscore[3] = {0};
uint8_t difficulty = 0;

int randomValue = 0;

double sqroot(double square)
{
    double root=square/3;
    int i;
    if (square <= 0) return 0;
    for (i=0; i<32; i++)
        root = (root + square / root) / 2;
    return root;
}

char* combineString(char str1[], char str2[]){
	//int length = sizeof(str1) + sizeof(str2);
	int i, j;
	char joinedString[16];
	for(i = 0; i < sizeof(str1); i++){
		joinedString[i] = str1[i];
	}
	for(j = 0; j < sizeof(str2); j++){
		joinedString[i + j] = str2[i];
	}
	return joinedString;
}

void collision(struct point* ball, struct paddle paddle1, struct paddle paddle2){
	if((int)ball->x == paddle1.x + PADDLE_WIDTH/2 + BALL_SIZE/2 && 
	ball->y - BALL_SIZE/2  <= paddle1.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 >= paddle1.y - PADDLE_HEIGHT/2 && ball->xSpeed < 0){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
		if(ball->ySpeed > 0.8){
			ball->ySpeed = 0.8;
		}
		if(ball->ySpeed < -0.8){
			ball->ySpeed = -0.8;
		}
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
		randomValue = rand() % 20;
	}

	if(ball->x < paddle1.x + PADDLE_WIDTH/2 + BALL_SIZE/2 && ball->x > paddle1.x - PADDLE_WIDTH/2 - BALL_SIZE/2 &&
	ball->y - BALL_SIZE/2 + 1  <= paddle1.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 - 1 >= paddle1.y - PADDLE_HEIGHT/2 && ball->xSpeed < 0){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/2;
		if(ball->ySpeed > 0.96){
			ball->ySpeed = 0.96;
		}
		if(ball->ySpeed < -0.96){
			ball->ySpeed = -0.96;
		}
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
		randomValue = rand() % 20;
	}

	if((int)ball->x == paddle2.x - PADDLE_WIDTH/2 - BALL_SIZE/2 && 
	ball->y - BALL_SIZE/2  <= paddle2.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 >= paddle2.y - PADDLE_HEIGHT/2 && ball->xSpeed > 0){
			ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
			if(ball->ySpeed > 0.8){
				ball->ySpeed = 0.8;
			}
			if(ball->ySpeed < -0.8){
			ball->ySpeed = -0.8;
		}
			ball->xSpeed = - sqroot(1 - ball->ySpeed * ball->ySpeed);
	}

	if(ball->x < paddle2.x - PADDLE_WIDTH/2 - BALL_SIZE/2 && ball->x > paddle2.x + PADDLE_WIDTH/2 + BALL_SIZE/2 &&
	ball->y - BALL_SIZE/2 + 1  <= paddle2.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 - 1 >= paddle2.y - PADDLE_HEIGHT/2 && ball->xSpeed > 0){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle2.y) * 1/2;
		if(ball->ySpeed > 0.96){
			ball->ySpeed = 0.96;
		}
		if(ball->ySpeed < -0.9){
			ball->ySpeed = -0.9;
		}
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
	}

	if(ball->y >= 31 - BALL_SIZE/2){
		ball->ySpeed = - sqrt(ball->ySpeed * ball->ySpeed);
	}
	if(ball->y <= 0 + BALL_SIZE/2){
		ball->ySpeed = sqrt(ball->ySpeed * ball->ySpeed);
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
	int i, j;
	for(i = paddle.y - PADDLE_HEIGHT/2; i <= paddle.y + PADDLE_HEIGHT/2; i++){
			for(j = paddle.x - PADDLE_WIDTH/2; j <= paddle.x + PADDLE_WIDTH/2; j++){
				renderPoint(j, i);
			}
			
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

	score_p1 = 3;
	score_p2 = 3;

	if(gameState == STATE_SINGLEPLAYER){
		
		while(score_p2 < 3){
			clearScreen();
			if(getbtns() & BTN3_MASK && paddle1.y < PADDLE_MAX){
				paddle1.y++;
			}

			if(getbtns() & BTN4_MASK && paddle1.y > PADDLE_MIN){
				paddle1.y--;
			}

			if(ball.xSpeed > 0 && paddle2.y + randomValue < ball.y && paddle2.y < PADDLE_MAX && ball.x > 70){
				paddle2.y++;
			}

			if(ball.xSpeed > 0 && paddle2.y + randomValue > ball.y && paddle2.y > PADDLE_MIN && ball.x > 70){
				paddle2.y--;
			}
			collision(&ball, paddle1, paddle2);
			ball.x += ball.xSpeed;
			ball.y += ball.ySpeed;
			renderPaddle(paddle1);
			renderPaddle(paddle2);
			renderBall(ball);
			updateScreen();
			delay(120000);
		}

		if(score_p1 > highscore[difficulty]){
			char name[4]={65, 65, 65, 0};
			uint8_t index = 0;
			uint8_t btnPressed = 0;
			char scoreString[10];
			sprintf(scoreString, "%d", score_p1);
			while (index < 3)
			{
				if(getbtns() & BTN4_MASK && !btnPressed){
					name[index]++;
					if(name[index] > 90){
						name[index] = 65;
					}
				}

				if(getbtns() & BTN3_MASK && !btnPressed){
					name[index]--;
					if(name[index] < 65){
						name[index] = 90;
					}
				}

				if(getbtns() & BTN2_MASK && !btnPressed && index > 0){
					index--;
				}

				if(getbtns() & BTN1_MASK && !btnPressed && index < 3){
					index++;
				}
				
				if(getbtns()){
					btnPressed = 1;
				}
				else{
					btnPressed = 0;
				}
				display_string(0, "HIGHSCORE:");
				display_string(1, combineString("Score: ", scoreString));
				display_string(2, "Enter name:");
				display_string(3, name);
				display_update();
				
			}
			

		}
	}

	if(gameState == STATE_MULTIPLAYER){
		
		while(score_p1 < 5 && score_p2 < 5){
			clearScreen();
			if(getbtns() & BTN1_MASK && paddle2.y < PADDLE_MAX){
				paddle2.y++;
			}

			if(getbtns() & BTN2_MASK && paddle2.y > PADDLE_MIN){
				paddle2.y--;
			}

			if(getbtns() & BTN3_MASK && paddle1.y < PADDLE_MAX){
				paddle1.y++;
			}

			if(getbtns() & BTN4_MASK && paddle1.y > PADDLE_MIN){
				paddle1.y--;
			}

			collision(&ball, paddle1, paddle2);
			ball.x += ball.xSpeed;
			ball.y += ball.ySpeed;
			renderPaddle(paddle1);
			renderPaddle(paddle2);
			renderBall(ball);
			updateScreen();
			delay(120000);
		}

	}	
	gameState = STATE_MENU;
}

void menu(void){
	PORTECLR = ~0x0;
	display_string(0, "MENU:");
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