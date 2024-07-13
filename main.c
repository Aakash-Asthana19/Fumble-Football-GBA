#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "gba.h"

#include "images/ball.h"
#include "images/player.h"
#include "images/goalImage.h"
#include "images/startScreen.h"
#include "images/ScoreScreen.h"
#include "images/winScreen.h"
#include "images/loseScreen.h"


enum gba_state {
  START, //first screen (goes to play)
  PREPLAY,
  PLAY, //play screen w/ player, ball, green field (goes to win or lose)
  WIN, //has the win screen (goes to score)
  PRESCORE,
  SCORE, //has the score screen w/ best time and your time (goes to start)
  LOSE, //has the game over screen (goes to score)
};

int main(void)
{
  Pic playerI = {80, 80, 10, 20, player};
  Pic footballI = {120, 200, 20, 10, Ball};
  Pic goalImageI = {60, 55, 20, 40, goalImage};

  REG_DISPCNT = MODE3 | BG2_ENABLE;
  //videoBuffer[80 * 240 + 120] = 0x1F00; // should set the middle pixel to green (WORKS!!)

  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  enum gba_state state = START;

  int secondsTime = 0; //holds the seconds to complete
  int bestTime = 17; //start at 17 to guarentees to get change
  int timerValue = 900; // Timer value in frames (15 seconds)
  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons

    switch (state) {
      case START:

        waitForVBlank();
        drawFullScreenImageDMA(StartScreen);
        drawImageDMA(goalImageI.x, goalImageI.y, 20, 40, goalImage); // draws the goal image top left

        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) { //if hit backspace key, go back to first screen
          state = START;
        } //if

        if (KEY_DOWN(BUTTON_START, currentButtons)) { //if hit enter key, go back to game screen
          state = PREPLAY;
        } //if

      //animate trophy going up and down
      static int direction = 1;
      goalImageI.x += direction;
      if (goalImageI.x <= 58 || goalImageI.x + goalImageI.height >= 160) {
          direction *= -1; //reverse direction
      }

        timerValue = 900; // reset timer value after each run
        secondsTime = 0; //reset secondsScore
        // state = ?
        break;

      case PREPLAY:

        waitForVBlank();
        fillScreenDMA(GREEN);
        drawImageDMA(playerI.x, playerI.y, 10, 20, player);  // draws the football player (should be middle left)
        drawImageDMA(footballI.x, footballI.y, 20, 10, Ball); //draws the football (should be bottom right)
        drawRectDMA(130, 200, 20, 30, YELLOW); // draws below the football, the column

        if (KEY_DOWN(BUTTON_START, currentButtons))
        {
          state = PLAY;
        }
      if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
        state = START;
      }
      break;


      case PLAY:

        waitForVBlank();
        drawRectDMA(0, 0, 240, 20, GRAY); // draws sideline at top
        //drawRectDMA(20, 50, 15, 15, WHITE);

        // timer stuff
        if (timerValue > 0) {
          timerValue--;
          secondsTime++;
          char timerText[10];
          snprintf(timerText, 10, "Time: %d", timerValue / 60);
          drawString(5, 20, timerText, WHITE); //draw timer
        } else {
          //15 seconds past
          playerI.x = 80; //put player back in og spot for next run
          playerI.y = 80;
          state = LOSE;
        }

        //collisions logic
        if (playerI.x <= 20 || playerI.x >= 140 || playerI.y <= 0 || playerI.y >= 230) { //if player goes out of bounds, lose game
          playerI.x = 80; //put player back in og spot for next run
          playerI.y = 80;
          state = LOSE;
        }

        int hitboxOffsetX = 0;  // if decrease, can go lower from top
        int hitboxOffsetY = -10; //hitbox 10 pixels higher now
        int hitboxWidth = playerI.width + 10;
        int hitboxHeight = playerI.height + 0;

        //player trophy collision
        if ((playerI.x + hitboxOffsetX + hitboxWidth) >= footballI.x && 
            (playerI.x + hitboxOffsetX) <= (footballI.x + footballI.width) &&
            (playerI.y + hitboxOffsetY + hitboxHeight) >= footballI.y &&
            (playerI.y + hitboxOffsetY) <= (footballI.y + footballI.height)) {

            playerI.x = 80; // Put player back to the original spot for the next run
            playerI.y = 80;            
            state = WIN;
        }


        //buttons logic
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) { //hit backspace so go back to start screen
          state = START;
        }
        if (KEY_DOWN(BUTTON_LEFT, BUTTONS)) { //move player left
          drawRectDMA(playerI.x, playerI.y, 10, 20, GREEN); // fill in former player position w/ green
          playerI.y -=2;
          drawImageDMA(playerI.x, playerI.y, 10, 20, player);
        }
        if (KEY_DOWN(BUTTON_RIGHT, BUTTONS)) { //move player right
          drawRectDMA(playerI.x, playerI.y, 10, 20, GREEN); //fill in former player position w/ green
          playerI.y +=2;
          drawImageDMA(playerI.x, playerI.y, 10, 20, player);
        }
        if (KEY_DOWN(BUTTON_DOWN, BUTTONS)) {  //move player down
          drawRectDMA(playerI.x, playerI.y, 10, 20, GREEN); //fill in former player position w/ green
          playerI.x +=2;
          drawImageDMA(playerI.x, playerI.y, 10, 20, player);

        }
        if (KEY_DOWN(BUTTON_UP, BUTTONS)) {  //move player up
          drawRectDMA(playerI.x, playerI.y, 10, 20, GREEN); //fill in former player position w/ green
          playerI.x -=2;
          drawImageDMA(playerI.x, playerI.y, 10, 20, player);

        }

        // state = ?
        break;
      case WIN:
        waitForVBlank();
        drawFullScreenImageDMA(WinScreen);
      
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) { //hit backspace so go back to start screen
          state = START;
        }
        if (KEY_DOWN(BUTTON_START, currentButtons)) { //hit enter so go to score screen
          state = PRESCORE;
        }

        // state = ?
        break;

      case PRESCORE:

        waitForVBlank();
        drawFullScreenImageDMA(SCORESCREEN);

        if (15 - (15 - (secondsTime / 60)) < bestTime) {
            bestTime = (15 - (secondsTime / 60)); //update fastest time
            bestTime = 15 - bestTime;
        }

        char scoreText[20];
        snprintf(scoreText, 20, "TIME: %d", 15 - (15 - (secondsTime / 60)));
        drawString(130, 10, scoreText, RED); //current time

        char bestTimeText[20];
        snprintf(bestTimeText, 20, "BEST: %d", bestTime);
        drawString(20, 10, bestTimeText, RED); //best time at top of screen     

        if (KEY_DOWN(BUTTON_START, currentButtons))
        {
          state = SCORE;
        }
      if (KEY_DOWN(BUTTON_SELECT, currentButtons)) {
        state = START;
      }
      break;        

      case SCORE:

        waitForVBlank();
        char scoreText2[10];
        snprintf(scoreText2, 10, "BEST: %d", bestTime);
        //drawString(20, 10, scoreText2, RED); // Draw score text at top
        waitForVBlank();
    
      
        if (KEY_DOWN(BUTTON_SELECT, currentButtons)) { //hit backspace so go back to start screen
          state = START;
        }
        // state = ?
        break;
      case LOSE:

        waitForVBlank();
        drawFullScreenImageDMA(LoseScreen);
      
      if (KEY_DOWN(BUTTON_SELECT, currentButtons)) { //hit backspace so go back to start screen
        state = START;
      }

        // state = ?
        break;      
    }

    previousButtons = currentButtons; // Store the current state of the buttons
  }

  UNUSED(previousButtons); // You can remove this once previousButtons is used

  return 0;
}
