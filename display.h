#ifndef display_h
#define display_h
#include "Arduino.h"
#include "globalStash.h"
#include "defense.h"
#include "player.h"
#include "scoreKeeper.h"
#include "controller.h"
#include "audio.h"
#include <EEPROM.h>
#include <LiquidCrystal.h>

/****************************************************
   Display Class header - LCD 2x16
      defines a buffer to write to
      and custom bitmaps used
 ****************************************************/

class Display {

  public:
    void init(GlobalStash* globalStash, LiquidCrystal* l);
    void updatePlayer();
    void updateDefense();
    void updateField();
    void clearBuffer();
    void write();
    void writeSplash();
    void writeStatus();    
    void updateStatus();
    void writeScore();    
    void updateScore();
    void writeMenu();
    void updateMenu();
    bool exitMenu();
    bool checkMenuMove();
    bool displayingField();
    bool displayingScore();
    bool displayingStatus();
    bool displayingSplash();
    bool displayingMenu();
    void blinkDefManTackle();
    void setStartingPositions();
    void playSplashAnimation();
    void playPuntAnimation();
    void playFieldGoalAnimation();
    void playMissedFieldGoalAnimation();
    void play1stDownAnimation();
    void playTouchdownAnimation();
    void playSplashAnimationInit();
    void play1stDownAnimationInit();
    void playTouchdownAnimationInit();
    void playPuntAnimationInit();
    void playFieldGoalAnimationInit();
    void playMissedFieldGoalAnimationInit();

  private:
    void pWrite(char** buf);
    void pUpdateDefense(int skipMan);
    void footballAnimation( );
    void initFieldCustomChars();
    void playAnimationInit(byte mode); 
    void playAnimationLogic(byte mode, byte msg1, byte msg2);
    void setBrightContrast();

    unsigned long lastBlinkCheckMS = 0;
    unsigned long lastAnimationCheckMS = 0;
    const int blinkDelay = 400;
    bool blinkHiddenFlag = 0;
    bool animationToggle=0;
    int animationDir=1;
    unsigned char menuNum=0;
    bool menuEdit=0;
      
    LiquidCrystal* lcd;
    int contrastPin = 10;
    int brightPin = 9;
    GlobalStash* gs;
    char lastDisplay;

    // -- ptr to Char 2 x Array of Strings ( 16 + /0 char)
    // -- need the /0 for serial print debug
    char* gDisplayBuff[2] = {"Row0            "
                           , "Row1            "};
    // -- pointer to head so we can ref like displayBuff[row][col]
    char** displayBuff = gDisplayBuff;


    char* gStatusBuff[2] = {"|Down|Posn|ToGo|"
                           ,"|    |>35<| 10 |"};
    char** statusBuff = gStatusBuff;

    char* gScoreBuff[2] = {"| 99 |12.6| 99 |"
                          ,"|Home| Q1 |Away|"};
    char** scoreBuff = gScoreBuff;

    // LCD ASCI nums
    const byte playerChar = 0; // -- custom 0
    const byte playerDefRow1n2 = 1; // -- custom 1
    const byte playerDefRow2n1 = 2; // -- custom 2

    const byte defenseRow0 = 45; //-- dash
    const byte defenseRow1 = 3;  // -- custom 3
    const byte defenseRow1n2 = 4;// -- custom 4
    const byte defenseRow2 = 95; // -- underline

    const byte endZoneRow0 = 5; //-- custom 5
    const byte endZoneRow1 = 6; //-- custom 6

    // Bitmaps of custom chars for field -- don't pgmem as we switch these around a bit.
     const unsigned char  playerRow0BM[8]  = {
      0b00000,
      0b00000,
      0b00000,
      0b10101,
      0b00000,
      0b00000,
      0b00000,
      0b00000
    };
     const unsigned char  playerRow1BM[8]  = {
      0b00000,
      0b10101,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000
    };
     const unsigned char  playerRow2BM[8]  = {
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b00000,
      0b10101,
      0b00000
    };

    //-- Animation Data ----
    
    byte custChar[8][8] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}
                            ,{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};

    //--- object positions ---
    byte oStartLeft, oStartRight;
    byte oLeft, oTop , oRight, oBot ;

    //-- football
    byte fbLeft = 0, fbTop = 24, fbRight = 15, fbBot = 32;

    //-- man
    byte mLeft = 0, mTop = 20, mRight = 15, mBot = 36;


    unsigned int* ptrAnimObj; // pointer to active animation object                           




};
#endif
