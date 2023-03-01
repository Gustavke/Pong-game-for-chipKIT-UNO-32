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
#define MAX_BALL_YSPEED 0.8
#define MAX_BALL_YSPEED_EDGE_BOUNCE 0.96
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
volatile uint8_t gameAdvance;
uint16_t score_p1;
uint16_t score_p2;
int highscore[3] = {0};
char highscoreString[3][22];
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

void combineString(const char str1[], const char str2[], char targetString[]){
	int i = 0;
	int j = 0;

	while(str1[i]){
		targetString[i] = str1[i];
		i++;
	}

	while(str2[j]){
		targetString[i + j] = str2[j];
		j++;
	}

	targetString[i + j] = 0;
}

void collision(struct point* ball, struct paddle paddle1, struct paddle paddle2){
	if((int)ball->x == paddle1.x + PADDLE_WIDTH/2 + BALL_SIZE/2 && 
	ball->y - BALL_SIZE/2  <= paddle1.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 >= paddle1.y - PADDLE_HEIGHT/2 && ball->xSpeed < 0){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
		if(ball->ySpeed > MAX_BALL_YSPEED){
			ball->ySpeed = MAX_BALL_YSPEED;
		}
		if(ball->ySpeed < - MAX_BALL_YSPEED){
			ball->ySpeed = - MAX_BALL_YSPEED;
		}
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
		randomValue = rand() % (8 + difficulty);
	}

	if(ball->x < paddle1.x + PADDLE_WIDTH/2 + BALL_SIZE/2 && ball->x > paddle1.x - PADDLE_WIDTH/2 - BALL_SIZE/2 &&
	ball->y - BALL_SIZE/2 + 1  <= paddle1.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 - 1 >= paddle1.y - PADDLE_HEIGHT/2 && ball->xSpeed < 0){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/2;
		if(ball->ySpeed > MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = MAX_BALL_YSPEED_EDGE_BOUNCE;
		}
		if(ball->ySpeed < - MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = - MAX_BALL_YSPEED_EDGE_BOUNCE;
		}
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
		randomValue = rand() % (8 + difficulty);
	}

	if((int)ball->x == paddle2.x - PADDLE_WIDTH/2 - BALL_SIZE/2 && 
	ball->y - BALL_SIZE/2  <= paddle2.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 >= paddle2.y - PADDLE_HEIGHT/2 && ball->xSpeed > 0){
			ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
			if(ball->ySpeed > MAX_BALL_YSPEED){
				ball->ySpeed = MAX_BALL_YSPEED;
			}
			if(ball->ySpeed < - MAX_BALL_YSPEED){
			ball->ySpeed = - MAX_BALL_YSPEED;
		}
			ball->xSpeed = - sqroot(1 - ball->ySpeed * ball->ySpeed);
	}

	if(ball->x < paddle2.x - PADDLE_WIDTH/2 - BALL_SIZE/2 && ball->x > paddle2.x + PADDLE_WIDTH/2 + BALL_SIZE/2 &&
	ball->y - BALL_SIZE/2 + 1  <= paddle2.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 - 1 >= paddle2.y - PADDLE_HEIGHT/2 && ball->xSpeed > 0){
		ball->ySpeed = ball->ySpeed + (ball->y - paddle2.y) * 1/2;
		if(ball->ySpeed > MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = MAX_BALL_YSPEED_EDGE_BOUNCE;
		}
		if(ball->ySpeed < - MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = - MAX_BALL_YSPEED_EDGE_BOUNCE;
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

	score_p1 = 0;
	score_p2 = 0;

	while(getbtns());

	if(gameState == STATE_SINGLEPLAYER){
		
		display_string(0, "-----DIFFICULTY-----");
		display_string(1, "BTN4 - EASY");
		display_string(2, "BTN3 - NORMAL");
		display_string(3, "BTN2 - HARD");
		display_update();

		while(!getbtns()){
			if(getbtns() & BTN2_MASK){
				difficulty = 0;
				
			}

			if(getbtns() & BTN3_MASK){
				difficulty = 1;
				
			}

			if(getbtns() & BTN4_MASK){
				difficulty = 2;
				}
	}
		uint8_t count = 0;
		while(score_p2 < 3){
			count++;
			clearScreen();
			if(getbtns() & BTN3_MASK && paddle1.y < PADDLE_MAX){
				paddle1.y++;
			}

			if(getbtns() & BTN4_MASK && paddle1.y > PADDLE_MIN){
				paddle1.y--;
			}

			if(ball.xSpeed > 0 && paddle2.y + randomValue < ball.y && paddle2.y < PADDLE_MAX && ball.x > 70 && count > difficulty){
				paddle2.y++;
			}

			if(ball.xSpeed > 0 && paddle2.y + randomValue > ball.y && paddle2.y > PADDLE_MIN && ball.x > 70 && count > difficulty){
				paddle2.y--;
			}
			collision(&ball, paddle1, paddle2);
			ball.x += ball.xSpeed;
			ball.y += ball.ySpeed;
			renderPaddle(paddle1);
			renderPaddle(paddle2);
			renderBall(ball);
			PORTECLR = 1;
			while(!gameAdvance);
			gameAdvance = 0;
			updateScreen();
			if(count > 2){
				count = 0;
			}
		}

		if(score_p1 > highscore[difficulty]){
			char name[22] = {65,65,65,0};
			uint8_t index = 0;
			uint8_t btnPressed = 0;
			char scoreString[7];
			sprintf(scoreString, "%d", score_p1);
			char combinedString[22] = {0};
			char combinedString2[44] = {0};
			combineString("Score: ", scoreString, combinedString);
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
				display_string(1, combinedString);
				display_string(2, "ENTER NAME:");
				display_string(3, name);
				display_update();
				
			}
			//combineString(name, " - ", combinedString2);
			//combineString(combinedString, scoreString, highscoreString);
			//combineString("test", "string", highscoreString);
			highscore[difficulty] = score_p1;
			int i;
			if(difficulty == 0){
				char highscoreSubstring[21] = "HARD:   ";
				strcat(highscoreSubstring, name);
				strcat(highscoreSubstring, " - ");
				strcat(highscoreSubstring, scoreString);
				for(i = 0; i < 22; i++){
					highscoreString[0][i] = highscoreSubstring[i];
				}
			}
			if(difficulty == 1){
				char highscoreSubstring[21] = "NORMAL: ";
				strcat(highscoreSubstring, name);
				strcat(highscoreSubstring, " - ");
				strcat(highscoreSubstring, scoreString);
				for(i = 0; i < 22; i++){
					highscoreString[1][i] = highscoreSubstring[i];
				}
			}
			if(difficulty == 2){
				
				char highscoreSubstring[21] = "EASY:   ";
				strcat(highscoreSubstring, name);
				strcat(highscoreSubstring, " - ");
				strcat(highscoreSubstring, scoreString);
				for(i = 0; i < 22; i++){
					highscoreString[2][i] = highscoreSubstring[i];
				}
			}

			while(getbtns());
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
			while(!gameAdvance);
			gameAdvance = 0;
			updateScreen();
		}
		if(score_p1 > score_p2){
			renderWinner(1);
		}
		if(score_p1 < score_p2){
			renderWinner(2);
		}
		updateScreen();
		while(!getbtns());
		while(getbtns());

	}	
	gameState = STATE_MENU;
}
void viewHighscore(){
	while(getbtns());
	display_string(0, "DIFFI. NAME SCORE");
	display_string(1, highscoreString[2]);
	display_string(2, highscoreString[1]);
	display_string(3, highscoreString[0]);
	display_update();
	while(!getbtns());
	while(getbtns());
	gameState = STATE_MENU;
}
void menu(void){
	PORTECLR = ~0x0;
	display_string(0, "--------MENU--------");
	display_string(1, "BTN4 - SINGLEPLAYER");
	display_string(2, "BTN3 - MULTIPLAYER");
	display_string(3, "BTN2 - HIGHSCORE");
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
	updateScreen();
	while(!getbtns());
	while(getbtns());
	enable_interrupt();
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
			viewHighscore();
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
		gameAdvance++;
		
  }
}