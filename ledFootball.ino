#include <LiquidCrystal.h>
#include "globalStash.h"
#include "controller.h"
#include "player.h"
#include "display.h"
#include "defense.h"
#include "scoreKeeper.h"
#include "audio.h"

/*****************************************
   Declare Global Vars to allocate Memory
 *****************************************/
LiquidCrystal gLCD(4, 5, 8, 11, 12, 13);
Display gDisplay;
Controller gController;
Audio gAudio;

Player gPlayer;
Defense gDefense_0;
Defense gDefense_1;
Defense gDefense_2;
Defense gDefense_3;
Defense gDefense_4;

// Array of pointers for nice array notation defenseTeam[i]
Defense *gDefenseTeam[] = {&gDefense_0, &gDefense_1, &gDefense_2, &gDefense_3, &gDefense_4};

ScoreKeeper gScoreKeeper;

GlobalStash globalStash;

// Local Vars for Loop
unsigned long lastDefMoveCheckMS = 0;




/********************************************
  ---  SETUP  ---
********************************************/
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(5));

  //Set gameMode to "splash screen" = 7
  globalStash.gameMode = 7;

  // Stick our objects into the globalStash
  globalStash.controller = &gController;
  globalStash.player = &gPlayer;
  globalStash.display = &gDisplay;
  globalStash.defenseTeam = gDefenseTeam;
  globalStash.scoreKeeper = &gScoreKeeper;
  globalStash.audio = &gAudio;

  // -- Initialize Objects

  globalStash.display->init(&globalStash, &gLCD);
  globalStash.controller->init();
  

  globalStash.player->init(&globalStash);
  globalStash.defenseTeam[0]->init(&globalStash, 0);
  globalStash.defenseTeam[1]->init(&globalStash, 1);
  globalStash.defenseTeam[2]->init(&globalStash, 2);
  globalStash.defenseTeam[3]->init(&globalStash, 3);
  globalStash.defenseTeam[4]->init(&globalStash, 4);

  globalStash.scoreKeeper->init(&globalStash);


}


/********************************************
  --- LOOP  ----
********************************************/
void loop() {

  /* GameModes:
      0 = starting play
      1 = play active
      2 = show status
      3 = show score
      4 = touch down
      42 = touch down splash
      5 = tackle
      51 = tackle 1st down
      52 = 1st down splash
      6 = tackle Status start
      7 = start splash
      8 = kick
      9 = time ran out
      10 = menu
  */
  switch (globalStash.gameMode) {

    // -- TEST --
    case 99: {
      
       //globalStash.gameMode = ?;
       
       globalStash.scoreKeeper->updateFieldPosition(49);
       globalStash.scoreKeeper->testKick();

      break;
    }

    // -- Starting Play --
    case 0: {

        if(!globalStash.display->displayingField()) {
            globalStash.display->setStartingPositions();
        }

        if (globalStash.player->checkMove() || globalStash.player->checkMoveJoyStick() ) {
          globalStash.display->updatePlayer();
          globalStash.display->write();
          globalStash.gameMode = 1;
        }

        // can go to menu from here
        if (globalStash.controller->getButtonPressed()==5 && globalStash.scoreKeeper->getStatusDown() != 4) {
          globalStash.gameMode = 10;
        }


        break;
      }

    // -- Main Play --
    case 1: {
        globalStash.scoreKeeper->updateClock();

        if (globalStash.player->checkMove() || globalStash.player->checkMoveJoyStick() ) {
          if (millis() > (lastDefMoveCheckMS + ((globalStash.difficulty.dataValue[globalStash.difficulty.currentIndex])<<4) )) {
            for (int i = 0; i < 5; i++) {
              if (globalStash.defenseTeam[random(0, 5)]->checkMove()) {
                break;
              }
            }
            lastDefMoveCheckMS = millis();
          }
  
          // -- We clear the display buffer
          globalStash.display->clearBuffer();
  
          // -- Put the Endzones back into buffer
          globalStash.display->updateField();
  
  
          // -- Update buffer in case of movements
          globalStash.display->updatePlayer();
          globalStash.display->updateDefense();
  
          // -- now write buffer to screen
          globalStash.display->write();
        }

        // -- Did we press Status or Score ?
        if (globalStash.controller->getStatusPressed()) {
          globalStash.gameMode = 2;
        }
        if (globalStash.controller->getScorePressed()) {
          globalStash.gameMode = 3;
        }
        
        // can go to menu from here
        if (globalStash.controller->getButtonPressed()==5 && globalStash.scoreKeeper->getStatusDown() != 4) {
          globalStash.gameMode = 10;
        }

        break;
      }

    // -- Status Screen --
    case 2: {

        globalStash.scoreKeeper->stopClock();

        // if we press status button again we go back to play
        if (globalStash.controller->getStatusPressed()) {
          globalStash.gameMode = 1;
        }

        //Really only want to update screen on a change
        if (globalStash.scoreKeeper->checkStatusChange() || !globalStash.display->displayingStatus()) {
          globalStash.display->updateStatus();
          globalStash.display->writeStatus();
        }
        
        // can go to menu from here
        if (globalStash.controller->getButtonPressed()==5 && globalStash.scoreKeeper->getStatusDown() != 4) {
          globalStash.gameMode = 10;
        }

        break;
      }

    // -- Score Screen --
    case 3: {

        //globalStash.scoreKeeper->stopClock();
        globalStash.scoreKeeper->updateClock();

        // if we press score button again we go back to play
        if (globalStash.controller->getScorePressed()) {
          globalStash.gameMode = 1;
        }

        //Really only want to update screen on a change
        if (globalStash.scoreKeeper->checkScoreChange() || !globalStash.display->displayingScore()) {
          globalStash.display->updateScore();
          globalStash.display->writeScore();
        }

        // can go to menu from here
        if (globalStash.controller->getButtonPressed()==5 && globalStash.scoreKeeper->getStatusDown() != 4) {
          globalStash.gameMode = 10;
        }

        break;
      }

    // -- Time Ran Out --
    case 9: {

        // if we press score button again we go back to play ... if not end of game
        if ((globalStash.controller->getScorePressed()) && (globalStash.scoreKeeper->getQuarter() < 3)) {
           globalStash.scoreKeeper->updateQuarter();
           globalStash.gameMode = 0;
        }

        //Really only want to update screen on a change
        if (globalStash.scoreKeeper->checkScoreChange() || !globalStash.display->displayingScore()) {
          globalStash.display->updateScore();
          globalStash.display->writeScore();
        }
        break;
      }


    // -- Touchdown Score Screen --
    case 4: {

        // if we press score button again we go back to play
        if (globalStash.controller->getScorePressed()) {
          globalStash.gameMode = 0;
        }

        //Really only want to update screen on a change
        if (globalStash.scoreKeeper->checkScoreChange() || !globalStash.display->displayingScore()) {
          globalStash.scoreKeeper->stopClock();
          globalStash.display->updateScore();
          globalStash.display->writeScore();
        }
        break;
      }
      
    // -- TouchDown Splash --
    case 42: {

        globalStash.scoreKeeper->stopClock();

        // skip if play animations is off
        if(!(globalStash.animation.dataValue[globalStash.animation.currentIndex])) {
            globalStash.gameMode = 4;
            break;
        }

        if(!globalStash.display->displayingSplash()) {
          globalStash.display->playTouchdownAnimationInit();
          globalStash.audio->playSongCharge();
        }
      
        if (globalStash.controller->getStatusPressed() || globalStash.controller->getScorePressed()  ) {
          globalStash.gameMode = 4;
          //globalStash.audio->playSoundTweet(); // Just to stop song
        } else {
          globalStash.display->playTouchdownAnimation();
        }

        break;
      }

      

    // -- Tackle --
    case 5: {

        globalStash.scoreKeeper->stopClock();

        // -- blink the Def Man that did the tackle 
        // ... man number is in GS ... display checks there
        globalStash.display->blinkDefManTackle();


        // -- Did we press Status to start next down ?
        if (globalStash.controller->getStatusPressed()) {
          globalStash.gameMode = 6;
        }

       break;
      }
      

    // -- Tackle 1st Down --
    case 51: {

        globalStash.scoreKeeper->stopClock();

        // -- blink the Def Man that did the tackle 
        // ... man number is in GS ... display checks there
        globalStash.display->blinkDefManTackle();


        // -- Did we press Status to start next down ?
        if (globalStash.controller->getStatusPressed()) {
          globalStash.gameMode = 52;
        }

       break;
      }

    // -- 1st Down Splash Screen --
    case 52: {

        // skip if play animations is off
        if(!(globalStash.animation.dataValue[globalStash.animation.currentIndex])) {
            globalStash.gameMode = 6;
            break;
        }

        if(!globalStash.display->displayingSplash()) {
          globalStash.display->play1stDownAnimationInit();
          globalStash.audio->playSongThunder();
        }
      
        if (globalStash.controller->getStatusPressed() ) {
          globalStash.gameMode = 6;
          //globalStash.audio->playSoundTweet(); // Just to stop song
        } else {
          globalStash.display->play1stDownAnimation();
        }

        break;
      }
      
    // -- Status Screen after Tackle --  
    case 6: {

        //Really only want to update screen on a change
        if (globalStash.scoreKeeper->checkStatusChange() || !globalStash.display->displayingStatus()) {
          globalStash.display->updateStatus();
          globalStash.display->writeStatus();
        }

        // if we press status button again we go back to start play
        if (globalStash.controller->getStatusPressed()) {
          globalStash.display->setStartingPositions();
          globalStash.gameMode = 0;
        }

        break;
      }
    // -- Splash Screen --
    case 7: {

        // skip if play animations is off
        if(!(globalStash.animation.dataValue[globalStash.animation.currentIndex])) {
            globalStash.display->setStartingPositions();
            globalStash.gameMode = 0;
            break;
        }


        if(!globalStash.display->displayingSplash()) {
          globalStash.display->playSplashAnimationInit();
          globalStash.audio->playSongMrTouchDown();
        }
      
        if (globalStash.player->checkMove() || globalStash.player->checkMoveJoyStick() ) {
          globalStash.display->setStartingPositions();
          globalStash.gameMode = 0;
          //globalStash.audio->playSoundTweet();
        } else {
          globalStash.display->playSplashAnimation();
        }

        break;
      }
    // -- Kick --
    case 8: {

        // skip if play animations is off
        if(!(globalStash.animation.dataValue[globalStash.animation.currentIndex])) {
            globalStash.gameMode = 0;
            break;
        }


        if(!globalStash.display->displayingSplash()) {
          if(globalStash.scoreKeeper->kickIsFieldGoal() ) {
            if(globalStash.scoreKeeper->fieldGoalIsGood() ) {
                globalStash.display->playFieldGoalAnimationInit();
                globalStash.audio->playSongRock();
            } else {
                globalStash.display->playMissedFieldGoalAnimationInit();
                globalStash.audio->playSoundTweetX2();
            }
          } else {
            globalStash.display->playPuntAnimationInit();
            globalStash.audio->playSoundTweetX2();
          }
          
        }

        // --- pressing score or status gets you out
        if (globalStash.controller->getStatusPressed() || globalStash.controller->getScorePressed()  ) {
          globalStash.gameMode = 0;
        } else {
          if(globalStash.scoreKeeper->kickIsFieldGoal() ) {
            if(globalStash.scoreKeeper->fieldGoalIsGood() ) {
                globalStash.display->playFieldGoalAnimation();
            } else {
                globalStash.display->playMissedFieldGoalAnimation();
            }
          } else {
            globalStash.display->playPuntAnimation();
          }
        }

        break;
      }
    // -- Menu --
    case 10: {

        // update 1st time thru
        if(!globalStash.display->displayingMenu()) {
          globalStash.scoreKeeper->stopClock();
          globalStash.display->updateMenu();
          globalStash.display->writeMenu();
        }

        // update if there is a move
        if(globalStash.display->checkMenuMove()) {
          globalStash.display->updateMenu();
          globalStash.display->writeMenu();
        }

        // new mode if we are on exitMenu
        if(globalStash.display->exitMenu()) {
          globalStash.gameMode = 0;
        }


        break;
      }
  }

}
