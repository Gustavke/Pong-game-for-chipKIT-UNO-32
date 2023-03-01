#include <stdio.h>
#include <errno.h>
#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>
#include "inputs.h"
#include <string.h>

// Game constants
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

// Game structs
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

// Global variables
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

	// Paddle 1 collision
	if((int)ball->x == paddle1.x + PADDLE_WIDTH/2 + BALL_SIZE/2 && 
	ball->y - BALL_SIZE/2  <= paddle1.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 >= paddle1.y - PADDLE_HEIGHT/2 && ball->xSpeed < 0){
		
		// Calculate ySpeed after collision
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;

		//Limit ySpeed if over max
		if(ball->ySpeed > MAX_BALL_YSPEED){
			ball->ySpeed = MAX_BALL_YSPEED;
		}
		if(ball->ySpeed < - MAX_BALL_YSPEED){
			ball->ySpeed = - MAX_BALL_YSPEED;
		}

		// Calculate xSpeed after collision
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);

		// Set new random value for AI paddle offset
		randomValue = rand() % (8 + difficulty);
	}

	// Paddle 1 edge collision
	if(ball->x < paddle1.x + PADDLE_WIDTH/2 + BALL_SIZE/2 && ball->x > paddle1.x - PADDLE_WIDTH/2 - BALL_SIZE/2 &&
	ball->y - BALL_SIZE/2 + 1  <= paddle1.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 - 1 >= paddle1.y - PADDLE_HEIGHT/2 && ball->xSpeed < 0){

		// Calculate ySpeed after collision
		ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/2;

		//Limit ySpeed if over max
		if(ball->ySpeed > MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = MAX_BALL_YSPEED_EDGE_BOUNCE;
		}
		if(ball->ySpeed < - MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = - MAX_BALL_YSPEED_EDGE_BOUNCE;
		}

		// Calculate xSpeed after collision
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
		
		// Set new random value for AI paddle offset
		randomValue = rand() % (8 + difficulty);
	}

	// Paddle 2 collision
	if((int)ball->x == paddle2.x - PADDLE_WIDTH/2 - BALL_SIZE/2 && 
	ball->y - BALL_SIZE/2  <= paddle2.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 >= paddle2.y - PADDLE_HEIGHT/2 && ball->xSpeed > 0){

			// Calculate ySpeed after collision
			ball->ySpeed = ball->ySpeed + (ball->y - paddle1.y) * 1/6;
			
			//Limit ySpeed if over max
			if(ball->ySpeed > MAX_BALL_YSPEED){
				ball->ySpeed = MAX_BALL_YSPEED;
			}
			if(ball->ySpeed < - MAX_BALL_YSPEED){
			ball->ySpeed = - MAX_BALL_YSPEED;
			}
			
			// Calculate xSpeed after collision
			ball->xSpeed = - sqroot(1 - ball->ySpeed * ball->ySpeed);
	}

	// Paddle 2 edge collision
	if(ball->x < paddle2.x - PADDLE_WIDTH/2 - BALL_SIZE/2 && ball->x > paddle2.x + PADDLE_WIDTH/2 + BALL_SIZE/2 &&
	ball->y - BALL_SIZE/2 + 1  <= paddle2.y + PADDLE_HEIGHT/2 && ball->y + BALL_SIZE/2 - 1 >= paddle2.y - PADDLE_HEIGHT/2 && ball->xSpeed > 0){

		// Calculate ySpeed after collision
		ball->ySpeed = ball->ySpeed + (ball->y - paddle2.y) * 1/2;

		//Limit ySpeed if over max
		if(ball->ySpeed > MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = MAX_BALL_YSPEED_EDGE_BOUNCE;
		}
		if(ball->ySpeed < - MAX_BALL_YSPEED_EDGE_BOUNCE){
			ball->ySpeed = - MAX_BALL_YSPEED_EDGE_BOUNCE;
		}

		// Calculate xSpeed after collision
		ball->xSpeed = sqroot(1 - ball->ySpeed * ball->ySpeed);
	}

	// Top and bottom wall collision
	if(ball->y >= 31 - BALL_SIZE/2){
		ball->ySpeed = - sqroot(ball->ySpeed * ball->ySpeed);
	}
	if(ball->y <= 0 + BALL_SIZE/2){
		ball->ySpeed = sqroot(ball->ySpeed * ball->ySpeed);
	}

	// Player 1 wall collision
	if(ball->x < 1 + BALL_SIZE/2){

		// Set ball position to center of field and invert speed
		ball->x = 64;
		ball->y = 16;
		ball->xSpeed = - ball->xSpeed;
		ball->ySpeed = - ball->ySpeed;

		// Award score to player 2
		score_p2++;

		// If multiplayer light up LED to represent player 2 score
		if(gameState == STATE_MULTIPLAYER){
			PORTESET = 1 << (score_p2 - 1);
		}
		// If singleplayer turn off LED to represent player 1 life lost
		else if(gameState == STATE_SINGLEPLAYER){
			PORTECLR = 1 << (4 + score_p2);
		}
	}

	// Player 2 wall collision
	if(ball->x > 127 - BALL_SIZE/2){

		// Set ball position to center of field and invert speed
		ball->x = 64;
		ball->y = 16;
		ball->xSpeed = - ball->xSpeed;
		ball->ySpeed = - ball->ySpeed;

		// Award score to player 1
		score_p1++;

		// If multiplayer light up LED to represent player 1 score
		if(gameState == STATE_MULTIPLAYER){
			PORTESET = 1 << (7 - (score_p1 - 1));
		}
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
	//Initialize ball and paddles
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

	// Initialize scores
	score_p1 = 0;
	score_p2 = 0;

	// Wait until all buttons are released before continuing
	while(getbtns());

	// If in single player mode, ask player to select difficulty
	if(gameState == STATE_SINGLEPLAYER){

		// Display difficulty options
		display_string(0, "-----DIFFICULTY-----");
		display_string(1, "BTN4 - EASY");
		display_string(2, "BTN3 - NORMAL");
		display_string(3, "BTN2 - HARD");
		display_update();

		// Wait for user to select difficulty
		while(1){
			if(getbtns() & BTN2_MASK){
				difficulty = 0;
				break;
			}

			if(getbtns() & BTN3_MASK){
				difficulty = 1;
				break;
			}

			if(getbtns() & BTN4_MASK){
				difficulty = 2;
				break;
			}
		}

		// Turn on 3 LEDS to indicate player lives
		PORTESET = 0xe0;

		// Initialize counter variable to regulate the speed of the AI paddle
		uint8_t count = 0;

		// Keep playing until AI wins 3 rounds
		while(score_p2 < 3){
			count++;

			// Clear screen and update paddle position
			clearScreen();
			if(getbtns() & BTN3_MASK && paddle1.y < PADDLE_MAX){
				paddle1.y++;
			}

			if(getbtns() & BTN4_MASK && paddle1.y > PADDLE_MIN){
				paddle1.y--;
			}

			// Update AI paddle position
			if(ball.xSpeed > 0 && paddle2.y + randomValue < ball.y && paddle2.y < PADDLE_MAX && ball.x > 70 && count > difficulty){
				paddle2.y++;
			}

			if(ball.xSpeed > 0 && paddle2.y + randomValue > ball.y && paddle2.y > PADDLE_MIN && ball.x > 70 && count > difficulty){
				paddle2.y--;
			}

			// Detect ball-paddle/-wall collisions and update ball position
			collision(&ball, paddle1, paddle2);
			ball.x += ball.xSpeed;
			ball.y += ball.ySpeed;

			// Render paddles and ball
			renderPaddle(paddle1);
			renderPaddle(paddle2);
			renderBall(ball);

			// Wait for gameAdvance signal before continuing
			while(!gameAdvance);
			gameAdvance = 0;
			updateScreen();

			// Reset the counter variable every 3 iterations
			if(count > 2){
				count = 0;
			}
		}

		// If player has beaten the previous high score, ask for their name and update high score
		if(score_p1 > highscore[difficulty]){

			// Initialize variables to allow the player to enter their name and store their high score.
			char name[22] = {65,65,65,0};
			uint8_t index = 0;
			uint8_t btnPressed = 0;
			char scoreString[7];
			sprintf(scoreString, "%d", score_p1);
			char combinedString[22] = {0};
			char combinedString2[44] = {0};
			combineString("SCORE: ", scoreString, combinedString);

			// Loop until 3 characters have been entered
			while (index < 3)
			{	
				// If player presses BTN4, increment current character value
				if(getbtns() & BTN4_MASK && !btnPressed){
					name[index]++;
					if(name[index] > 90){
						name[index] = 65;
					}
				}

				// If player presses BTN3, decrement current character value
				if(getbtns() & BTN3_MASK && !btnPressed){
					name[index]--;
					if(name[index] < 65){
						name[index] = 90;
					}
				}

				// If player presses BTN2, move to previous character
				if(getbtns() & BTN2_MASK && !btnPressed && index > 0){
					index--;
				}

				// If player presses BTN1, move to next character
				if(getbtns() & BTN1_MASK && !btnPressed && index < 3){
					index++;
				}
				
				if(getbtns()){
					btnPressed = 1;
				}
				else{
					btnPressed = 0;
				}

				// Display highscore and current name value while it is being entered by the player
				display_string(0, "HIGHSCORE:");
				display_string(1, combinedString);
				display_string(2, "ENTER NAME:");
				display_string(3, name);
				display_update();
				
			}
			// Store highscore for current difficulty
			highscore[difficulty] = score_p1;

			// Save name and score in string format to display in highscore view
			int i;
			char highscoreSubstring[21] = {0};
			if(difficulty == 0){
				strcat(highscoreSubstring, "HARD:   ");
			}
			else if(difficulty == 1){
				strcat(highscoreSubstring, "NORMAL: ");
			}
			else if(difficulty == 2){
				strcat(highscoreSubstring, "EASY:   ");
			}

			strcat(highscoreSubstring, name);
			strcat(highscoreSubstring, " - ");
			strcat(highscoreSubstring, scoreString);
			for(i = 0; i < 22; i++){
				highscoreString[difficulty][i] = highscoreSubstring[i];
			}
			// Wait until all buttons are released before continuing
			while(getbtns());
		}
		// If player have not beaten the previous highscore, display game over image
		else{
			renderGameOver();
			updateScreen();

			// Wait until any button has been pressed and then untill all butons are released before continuing
			while(!getbtns());
			while(getbtns());
		}
	}
	// Multiplayer mode
	else if(gameState == STATE_MULTIPLAYER){
		
		// Keep playing until either player has won 5 rounds
		while(score_p1 < 5 && score_p2 < 5){

			// Clear screen and update paddle position
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

			// Detect ball-paddle/-wall collisions and update ball position
			collision(&ball, paddle1, paddle2);
			ball.x += ball.xSpeed;
			ball.y += ball.ySpeed;

			// Render paddles and ball
			renderPaddle(paddle1);
			renderPaddle(paddle2);
			renderBall(ball);

			// Wait for gameAdvance signal before continuing
			while(!gameAdvance);
			gameAdvance = 0;
			updateScreen();
		}

		// Render win screen for winning player
		if(score_p1 > score_p2){
			renderWinner(1);
		}
		if(score_p1 < score_p2){
			renderWinner(2);
		}
		updateScreen();

		// Wait until any button has been pressed and then until all buttons are released before continuing
		while(!getbtns());
		while(getbtns());

	}	
	// Set game state to return to menu
	gameState = STATE_MENU;
}

void viewHighscore(){
	// Wait until all butons are released before continuing
	while(getbtns());

	// Display high score string with name and score for each difficulty
	display_string(0, "DIFFI.  NAME SCORE");
	display_string(1, highscoreString[2]);
	display_string(2, highscoreString[1]);
	display_string(3, highscoreString[0]);
	display_update();

	// Wait until any button has been pressed and then until all buttons are released before continuing
	while(!getbtns());
	while(getbtns());

	// Set game state to return to menu
	gameState = STATE_MENU;
}

void menu(void){
	// Turn off all LEDS
	PORTECLR = ~0x0;
	// Display menu items
	display_string(0, "--------MENU--------");
	display_string(1, "BTN4 - SINGLEPLAYER");
	display_string(2, "BTN3 - MULTIPLAYER");
	display_string(3, "BTN2 - HIGHSCORE");
	display_update();

	// Loop until player selects a menu item with a button
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

	// Initialize IO
	init();
	display_init();
	enable_interrupt();

	// Show splash screen until any button has been pressed and released
	updateScreen();
	while(!getbtns());
	while(getbtns());

	// Set rand seed to the value of the timer at the moment the button was pressed and released
	srand(TMR2);

	// Enter main loop that calls functions for each section of the game
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

	return 0;
} 


void user_isr( void ) {
	
	if(IFS(0) & 0x100){
		IFSCLR(0) = 0x100;
		gameAdvance++;
		
  }
}