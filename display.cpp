/*****************************************************
   Class controls the "display"  this version deals with
   a 16x2 LCD ... also assumes the LCD has 80 memory locations
   and 8 custom chars
 *****************************************************/
#include "display.h"


/******************************************************
   init()
      Get pointer to GlobalStash
      Set initial player position
      Create Custom chars for player
******************************************************/
void Display::init(GlobalStash* globalStash, LiquidCrystal* l) {

  // Store gs pointer and setup LCD
  gs = globalStash;
  lcd = l;

  // Load menu from EEprom if 1st 2 bytes contain magic number
  byte mn1, mn2;
  EEPROM.get(0, mn1);
  EEPROM.get(1, mn2);
  if(mn1==128 && mn2==64) {
      EEPROM.get(2, gs->animation.currentIndex);
      EEPROM.get(3, gs->brightness.currentIndex);
      EEPROM.get(4, gs->contrast.currentIndex);
      EEPROM.get(5, gs->difficulty.currentIndex);
      EEPROM.get(6, gs->shortGame.currentIndex);
      EEPROM.get(7, gs->volume.currentIndex);
  }

  pinMode(contrastPin, OUTPUT);
  pinMode(brightPin, OUTPUT);
  setBrightContrast();

  lcd->begin(16, 2);
  lcd->clear();
  initFieldCustomChars();


}

void Display::setBrightContrast() {
  analogWrite(brightPin,gs->brightness.dataValue[gs->brightness.currentIndex]);
  analogWrite(contrastPin,gs->contrast.dataValue[gs->contrast.currentIndex]);
}

void Display::initFieldCustomChars() {


  //Create Player Custom Chars
  lcd->createChar(playerChar, playerRow0BM); // we change this around so it is not in pgmMem


  //--- Seem to need to put pgmMem into ram to call createChar()
  
  // -- Clear local mem
  memset (custChar[0], 0, sizeof(custChar[0])*8);
  
  // -- pull the pgmMem bytes line by line into local
  for(byte i=0;i<8;i++) {
    custChar[playerDefRow1n2][i]  = pgm_read_byte_near(playerDefRow1n2BM + i);
    custChar[playerDefRow2n1][i]  = pgm_read_byte_near(playerDefRow2n1BM + i);
    custChar[defenseRow1][i]  = pgm_read_byte_near(defenseRow1BM + i);
    custChar[defenseRow1n2][i]  = pgm_read_byte_near(defenseRow1n2BM + i);
    custChar[endZoneRow0][i]  = pgm_read_byte_near(endZoneRow0BM + i);
    custChar[endZoneRow1][i]  = pgm_read_byte_near(endZoneRow1BM + i);
  }
  
  

  // Shared player and Defense spot
  lcd->createChar(playerDefRow1n2, custChar[playerDefRow1n2]);
  lcd->createChar(playerDefRow2n1, custChar[playerDefRow2n1]);

  // Create Defense Custom Chars
  lcd->createChar(defenseRow1, custChar[defenseRow1]);
  lcd->createChar(defenseRow1n2, custChar[defenseRow1n2]);

  // Create Field Custom Chars
  lcd->createChar(endZoneRow0, custChar[endZoneRow0]);
  lcd->createChar(endZoneRow1, custChar[endZoneRow1]);

 

  // ---- Seperators for Score/Status ... never change
  statusBuff[0][0]=statusBuff[0][5]=statusBuff[0][10]=statusBuff[0][15]=255;
  statusBuff[1][0]=statusBuff[1][5]=statusBuff[1][10]=statusBuff[1][15]=255;
  scoreBuff[0][0]=scoreBuff[0][5]=scoreBuff[0][10]=scoreBuff[0][15]=255;
  scoreBuff[1][0]=scoreBuff[1][5]=scoreBuff[1][10]=scoreBuff[1][15]=255;
 
}

/****************************************************
  updatePlayer()
      Update buffer with correct bitmap depending
      on the row the player is on
*****************************************************/
void Display::updatePlayer() {
  switch (gs->player->fp.row) {
    case 0: {
        lcd->createChar(playerChar, playerRow0BM);
        displayBuff[0][gs->player->fp.col] = playerChar;
        break;
      }
    case 1: {
        // 32 is empty
        if(displayBuff[1][gs->player->fp.col] == 32) {
            lcd->createChar(playerChar, playerRow1BM);
            displayBuff[1][gs->player->fp.col] = playerChar;
        
        // share a spot
        } else {
            displayBuff[1][gs->player->fp.col] = playerDefRow1n2;            
        }
        break;
      }
    case 2: {
        // 32 is empty
        if(displayBuff[1][gs->player->fp.col] == 32) {
            lcd->createChar(playerChar, playerRow2BM);
            displayBuff[1][gs->player->fp.col] = playerChar;
        
        // share a spot
        } else {
           displayBuff[1][gs->player->fp.col] = playerDefRow2n1;            
        }
        break;
      }
  }
}


void Display::blinkDefManTackle() {

  if ((millis() > lastBlinkCheckMS + blinkDelay)) {
    if(!blinkHiddenFlag) {
      // -- Need to clear and re-update without defMan
      clearBuffer();
      updateField();
      updatePlayer();
      pUpdateDefense(gs->defManTackle);
    } else {
      // Put DefMan back
      clearBuffer();
      updateField();
      updatePlayer();
      pUpdateDefense(-1);
    }
    blinkHiddenFlag = !blinkHiddenFlag;
    lastBlinkCheckMS = millis();
    write();
  }
}



/****************************************************
  updateDefense()
*****************************************************/
void Display::updateDefense() {
  pUpdateDefense(-1);
}

void Display::pUpdateDefense(int hideMan) {
    for(int i=0;i<5;i++) {
      
      // skip hideMan
      if(hideMan==i) i++;
      if(i>4) return;
      
      switch(gs->defenseTeam[i]->fp.row) {
        case 0: {
          displayBuff[0][gs->defenseTeam[i]->fp.col] = defenseRow0;
          break;
        }
        case 1: {
          // 32 is empty
          if(displayBuff[1][gs->defenseTeam[i]->fp.col] == 32) {
              displayBuff[1][gs->defenseTeam[i]->fp.col] = defenseRow1;
          // share a spot
          } else {
            // 0-2 is a player bitmap
            if(displayBuff[1][gs->defenseTeam[i]->fp.col] > 2) {
              displayBuff[1][gs->defenseTeam[i]->fp.col] = defenseRow1n2 ;            
            }else {
              displayBuff[1][gs->defenseTeam[i]->fp.col] = playerDefRow2n1;            
            }
          }
          break;
        }
        case 2: {
          // 32 is empty
          if(displayBuff[1][gs->defenseTeam[i]->fp.col] == 32) {
              displayBuff[1][gs->defenseTeam[i]->fp.col] = defenseRow2;
          // share a spot
          } else {
            // 0-2 is a player bitmap
            if(displayBuff[1][gs->defenseTeam[i]->fp.col] > 2) {
              displayBuff[1][gs->defenseTeam[i]->fp.col] = defenseRow1n2 ;            
            }else {
              displayBuff[1][gs->defenseTeam[i]->fp.col] = playerDefRow1n2;            
            }
          }
          break;
        }

      }
      
    }


}


/****************************************************
  updateField()
      Paint the endzone BMs to the buffer
*****************************************************/
void Display::updateField() {

  // ---- End Zones
  displayBuff[0][0] = endZoneRow0;
  displayBuff[1][0] = endZoneRow1;
  displayBuff[0][1] = 255;
  displayBuff[1][1] = 255;

  displayBuff[0][15] = endZoneRow0;
  displayBuff[1][15] = endZoneRow1;
  displayBuff[0][14] = 255;
  displayBuff[1][14] = 255;

}

/****************************************************
  updateStatus()
 *****************************************************/
void Display::updateStatus() {

  int fp = gs->scoreKeeper->getFieldPosition();
  int fp10s = fp / 10;
  int fp1s = (fp - (fp10s * 10));

  //down -- 1-4
  statusBuff[1][2] = 48 + gs->scoreKeeper->getStatusDown();

  //Yardline -- 01 to 50  Ascii 48 = 0
  statusBuff[1][7] = 48 + fp10s;
  statusBuff[1][8] = 48 + fp1s;

  //Side of field -> 126 or <- 127 or 32 is blank
  byte ballDir = (gs->scoreKeeper->homeHasBall())?126:127;
  statusBuff[1][6] = (gs->scoreKeeper->ballIsOnHomeSide())?ballDir:32;
  statusBuff[1][9] = (gs->scoreKeeper->ballIsOnHomeSide())?32:ballDir;

  //ToGo -- 01 to >10  Ascii 48 is 0
  int tg = gs->scoreKeeper->getYardsToGo();
  int tg10s = tg / 10;
  int tg1s = (tg - (tg10s * 10));
  statusBuff[1][12] = 48 + tg10s;
  statusBuff[1][13] = 48 + tg1s;


}

/****************************************************
 *****************************************************/
void Display::updateScore() {
  int clockVal = gs->scoreKeeper->getClock();
  int time10s = clockVal / 100;
  int time1s = (clockVal - (time10s * 100)) / 10;
  int timeDec = clockVal - ((time10s * 100) + (time1s * 10));

  int s = gs->scoreKeeper->getHomeScore();
  int s10s = s / 10;
  int s1s = (s - (s10s * 10));
  if (s10s > 9) s10s = 9;

  //Home -- 00 to 99
  scoreBuff[0][2] = 48 + s10s;
  scoreBuff[0][3] = 48 + s1s;

  //Time -- 15.0 to 00.0
  scoreBuff[0][6] = 48 + time10s;
  scoreBuff[0][7] = 48 + time1s;
  scoreBuff[0][8] = 46;  // decimal point
  scoreBuff[0][9] = 48 + timeDec;


  s = gs->scoreKeeper->getAwayScore();
  s10s = s / 10;
  s1s = (s - (s10s * 10));
  if (s10s > 9) s10s = 9;

   //Away -- 00 to 99
  scoreBuff[0][12] = 48 + s10s;
  scoreBuff[0][13] = 48 + s1s;


  s = gs->scoreKeeper->getQuarter();
  scoreBuff[1][8] = 48 + s;

}

bool Display::checkMenuMove() {
  byte btn = gs->controller->getButtonPressed();
  byte newIndex;
  int incDec = -1;
  bool rtn = false;
  if(menuEdit && (btn==1 || btn==2 || btn==4)) {
    rtn = true;
    if(btn==1) {
      // left button
      menuEdit = !menuEdit;
    } else {
      if(btn==2) incDec = 1;

      // note: newIndex wraps to 255 if decremented less than zero
      switch (menuNum) {
        case 0: {
          newIndex = gs->animation.currentIndex + incDec;
          if (newIndex <= gs->animation.maxIndex) gs->animation.currentIndex = newIndex;
          break; }
        case 1: {
          newIndex = gs->brightness.currentIndex + incDec;
          if (newIndex <= gs->brightness.maxIndex) {
            gs->brightness.currentIndex = newIndex;
            setBrightContrast();
          }
          break; }
        case 2: {
          newIndex = gs->contrast.currentIndex + incDec;
          if (newIndex <= gs->contrast.maxIndex) {
            gs->contrast.currentIndex = newIndex;
            setBrightContrast();
          }
          break; }
        case 3: {
          newIndex = gs->difficulty.currentIndex + incDec;
          if (newIndex <= gs->difficulty.maxIndex) gs->difficulty.currentIndex = newIndex;
          break; }
        case 4: {
          newIndex = gs->shortGame.currentIndex + incDec;
          if (newIndex <= gs->shortGame.maxIndex) gs->shortGame.currentIndex = newIndex;
          break; }
        case 5: {
          newIndex = gs->volume.currentIndex + incDec;
          if (newIndex <= gs->volume.maxIndex){
            gs->volume.currentIndex = newIndex;
            Audio::audioVolume = gs->volume.dataValue[gs->volume.currentIndex];
          }
          break; }
      } 
    }
  } else {
    rtn = true;
    if(!menuEdit && (btn==3 || btn==2 || btn==4)) {
      if(btn==3) {
        // right button
        menuEdit = !menuEdit;
        
      } else { // up down move menu
        menuNum = (btn==2 && menuNum>0)?menuNum-1:menuNum;
        menuNum = (btn==4 && menuNum<6)?menuNum+1:menuNum;
      }      
    }
  }
  return rtn;
}

void Display::updateMenu() {

  clearBuffer();

  for(byte j=0;j<sizeof(splashMsg[0]);j++) {
    displayBuff[0][j] = pgm_read_byte_near(menu[menuNum] + j);
    displayBuff[1][j] = pgm_read_byte_near(menu[menuNum+1] + j);
  }

  // -> 126 or <- 127 ... around menu or value
  displayBuff[0][((!menuEdit)?0:13)] = 126;
  displayBuff[0][15] = (menuEdit)?127:32;

  switch (menuNum) {
    case 0: {
      displayBuff[0][14] = gs->animation.displayValue[gs->animation.currentIndex];
      displayBuff[1][14] = gs->brightness.displayValue[gs->brightness.currentIndex];
      break; }
    case 1: {
      displayBuff[0][14] = gs->brightness.displayValue[gs->brightness.currentIndex];
      displayBuff[1][14] = gs->contrast.displayValue[gs->contrast.currentIndex];
      break; }
    case 2: {
      displayBuff[0][14] = gs->contrast.displayValue[gs->contrast.currentIndex];
      displayBuff[1][14] = gs->difficulty.displayValue[gs->difficulty.currentIndex];
      break; }
    case 3: {
      displayBuff[0][14] = gs->difficulty.displayValue[gs->difficulty.currentIndex];
      displayBuff[1][14] = gs->shortGame.displayValue[gs->shortGame.currentIndex];
      break; }
    case 4: {
      displayBuff[0][14] = gs->shortGame.displayValue[gs->shortGame.currentIndex];
      displayBuff[1][14] = gs->volume.displayValue[gs->volume.currentIndex];
      break; }
    case 5: {
      displayBuff[0][14] = gs->volume.displayValue[gs->volume.currentIndex];
      //displayBuff[1][14] = EXIT;
      break; }
      
  }  
}

/******************************************************
  clearBuffer()
      Write spaces to the buffer row1 and row2
      Clear main screen (16 cols) or full buffer (32 cols)
******************************************************/
void Display::clearBuffer() {

  for (int i = 0; i < 16; i++) {
    displayBuff[0][i] = 32; // 32 is a space
    displayBuff[1][i] = 32;
  }


}
bool Display::displayingField() {
  if(lastDisplay == 'F') return true;
  return false;
}

bool Display::displayingScore() {
  if(lastDisplay == 'C') return true;
  return false;
}
bool Display::displayingStatus() {
  if(lastDisplay == 'T') return true;
  return false;
}
bool Display::displayingSplash() {
  if(lastDisplay == 'S') return true;
  return false;
}
bool Display::displayingMenu() {
  if(lastDisplay == 'M') return true;
  return false;
}

bool Display::exitMenu() {
  if(menuNum==6 && menuEdit) {

    // 1st 2 byte a magic number to know it is our data
    EEPROM.update(0, 128);
    EEPROM.update(1, 64);
    EEPROM.update(2, gs->animation.currentIndex);
    EEPROM.update(3, gs->brightness.currentIndex);
    EEPROM.update(4, gs->contrast.currentIndex);
    EEPROM.update(5, gs->difficulty.currentIndex);
    EEPROM.update(6, gs->shortGame.currentIndex);
    EEPROM.update(7, gs->volume.currentIndex);
  
    return true;
  }
  return false;
}



void Display::write() {
  lastDisplay = 'F';
  pWrite(displayBuff);
}

void Display::writeSplash() {
  lastDisplay = 'S';
  pWrite(displayBuff);
}

void Display::writeStatus() {
  lastDisplay = 'T';
  pWrite(statusBuff);
}
void Display::writeScore() {
  lastDisplay = 'C';
  pWrite(scoreBuff);
}
void Display::writeMenu() {
  lastDisplay = 'M';
  pWrite(displayBuff);
}
/******************************************************
  write()
    write 2 lines to LCD.
 ******************************************************/
void Display::pWrite(char** buf) {

  //-- row0 starts at 0 and goes to 39
  lcd->setCursor(0, 0);
  for (int i = 0; i < 16; i++) {
    lcd->write(buf[0][i]);
  }

  //-- row1 starts at 40 and goes to 79
  lcd->setCursor(40, 0);
  for (int i = 0; i < 16; i++) {
    lcd->write(buf[1][i]);
  }

}


void Display::setStartingPositions() {


  if(gs->scoreKeeper->homeHasBall()) {
    gs->player->setPosition(1, 4);
    gs->defenseTeam[0]->setPosition(0, 7);
    gs->defenseTeam[1]->setPosition(1, 7);
    gs->defenseTeam[2]->setPosition(2, 7);
    gs->defenseTeam[3]->setPosition(1, 8);
    gs->defenseTeam[4]->setPosition(1, 12);    
  } else {
    gs->player->setPosition(1, 11);
    gs->defenseTeam[0]->setPosition(0, 8);
    gs->defenseTeam[1]->setPosition(1, 8);
    gs->defenseTeam[2]->setPosition(2, 8);
    gs->defenseTeam[3]->setPosition(1, 7);
    gs->defenseTeam[4]->setPosition(1, 3);    
  }


  // -- Reset field display
  initFieldCustomChars();
  clearBuffer();
  updateField();
  updatePlayer();
  updateDefense();
  write();

}
                    
// *** Types of Animation Setup                        
void Display::playSplashAnimationInit() { playAnimationInit(1); }
void Display::play1stDownAnimationInit() {playAnimationInit(2); }
void Display::playTouchdownAnimationInit() {playAnimationInit(2); }
void Display::playPuntAnimationInit() { playAnimationInit(1); }
void Display::playFieldGoalAnimationInit() {  playAnimationInit(1); }
void Display::playMissedFieldGoalAnimationInit() { playAnimationInit(1); }

void Display::playAnimationInit(byte mode) {

    switch (mode) {
      
      // ball object
      case 1: {
        oStartLeft= fbLeft;  oStartRight = fbRight;
        oLeft=fbLeft;   oTop=fbTop;   oRight = fbRight;  oBot =fbBot;

        ptrAnimObj = ballChars;
        break;
      }

      // man object  
      case 2: {
        oStartLeft= mLeft;  oStartRight = mRight;
        oLeft=mLeft;   oTop=mTop;   oRight = mRight;  oBot =mBot;

        ptrAnimObj = man1;
        break;
      }
    }
}

//*** Animation Logic and messages

void Display::playSplashAnimation() {playAnimationLogic(1,1,2);}
void Display::playFieldGoalAnimation() {playAnimationLogic(1,7,0);}
void Display::playPuntAnimation() {playAnimationLogic(3,5,0);}
void Display::play1stDownAnimation() {playAnimationLogic(2,3,0);}
void Display::playTouchdownAnimation() {playAnimationLogic(2,4,0);}
void Display::playMissedFieldGoalAnimation() { playAnimationLogic(3,6,7);}

void Display::playAnimationLogic(byte mode, byte msg1, byte msg2) {
  
  for(byte j=0;j<sizeof(splashMsg[0]);j++) {
    displayBuff[0][j] = pgm_read_byte_near(splashMsg[msg1] + j);
    displayBuff[1][j] = pgm_read_byte_near(splashMsg[msg2] + j);
  }

    switch (mode) {
      
      // basic left to right
      case 1: {
          oLeft++;
          oRight++;
        
          // --- wrap back
          if(oLeft > 120) {
            oLeft = oStartLeft;
            oRight = oStartRight;
          }
 
          break;
      }

      // left to right toggle man
      case 2: {
          // delay without blocking
          if (lastAnimationCheckMS + 50 < millis()) {
              oLeft++;
              oRight++;
        
              ptrAnimObj = (animationToggle)?man1:man2;
              animationToggle = !animationToggle;
        
              // wrapp back
              if(oLeft > 120) {
                oLeft = oStartLeft;
                oRight = oStartRight;
              }
              lastAnimationCheckMS = millis();
          }        
         
         break;
      }

      // left to right bounce
      case 3: {
          oLeft++;
          oRight++;
        
          // --- wrap back
          if(oLeft > 120) {
            oLeft = oStartLeft;
            oRight = oStartRight;
          }
        
          oTop=oTop+ animationDir;
          oBot=oBot+ animationDir;
          //-- switch directions
          if(oTop<15 || oTop>40) {
            animationDir = animationDir * -1;
          }
         break;
      }
    }
     
  footballAnimation();
  writeSplash();
}





/************************************************
 * 
 */
void Display::footballAnimation( ) {
  byte custCharCnt = 0; // tracking how many we use ... up to 8
  byte oStart = 0;  // start row of object to copy
  byte sStart = 0;  // start row of square to copy to
  byte numRows = 0; // num of obj rows to copy to sqr
  bool shiftLeft = 0; // overlap direction left to right
  byte numCols = 0;   // cols to shift overlap

  // Local rect for square positions from pgmmem
  byte left, right, top, bot;
 
   

  // --- Collision Loop ---
  for(byte i=0;i<32;i++) {

    // bail if we ran out of cust chars
    if(custCharCnt>7) break;

    
    left = pgm_read_byte_near(sRect + (i<<2));
    top = pgm_read_byte_near(sRect + (i<<2)+1);
    right = pgm_read_byte_near(sRect + (i<<2)+2);
    bot = pgm_read_byte_near(sRect + (i<<2)+3);


    // --- Collision ---
    if(    (oLeft < right) // Not too right
        && (oRight > left) // not too left
        && (oTop < bot)    // not too low
        && (oBot > top))     // not too high
        {
          
          // -- Build Cust Char --
          
          // -- Clear it
          memset (custChar[custCharCnt], 0, sizeof(custChar[0]));

          // -- figure out clipping info
          oStart = (oTop<top)?top-oTop:0;
          sStart = (top<oTop)?oTop-top:0;
          numRows = (bot<oBot)?bot-(sStart+top):oBot-(oStart+oTop);
          shiftLeft = (right>oRight)?1:0;
          numCols = (shiftLeft)?right-oRight:oRight-right;

          // -- copy part of object to cust char --
          for(byte ii=0;ii<=numRows;ii++) {
            custChar[custCharCnt][ii+sStart] = (shiftLeft)? pgm_read_word_near(ptrAnimObj + oStart + ii)<<numCols
                                                          : pgm_read_word_near(ptrAnimObj + oStart + ii)>>numCols;
          }

          lcd->createChar(custCharCnt, custChar[custCharCnt]);
          
          // -- update buffer with special char & use it up
          if(i<16) {
            displayBuff[0][i] = custCharCnt++;
          } else {
            displayBuff[1][i-16] = custCharCnt++;
          }
        }
    
  }
  
}
