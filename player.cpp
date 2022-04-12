/*****************************************************
   Class of the player
   basically keeps track of position and how to move
 *****************************************************/
#include "player.h"

/******************************************************
   Get pointer to Globals
   Set initial player position
******************************************************/
void Player::init(GlobalStash* globalStash) {
  gs = globalStash;

}

void Player::setPosition(byte startRow, byte startCol) {
  fp.row = startRow;
  fp.col = startCol;

}


//**********************************************
//**********************************************
bool Player::checkMove() {
  byte btn = gs->controller->getButtonPressed();

  // kick
  if(btn==5 && gs->scoreKeeper->checkKick()) {
    return false;
  }
  
  return checkMoveWorker(btn);
}
bool Player::checkMoveJoyStick() {
  byte btn = gs->controller->getJoyStick();
  return checkMoveWorker(btn);
}

bool Player::checkMoveWorker(byte btn) {  

  displayPosition newFp;
  newFp.row = fp.row;
  newFp.col = fp.col;
  
  
  bool l2r = gs->scoreKeeper->homeHasBall();

  switch (btn) {
    // up
    case 2:
    case 12:
    case 32: {
        if (fp.row > 0) {
          newFp.row--;
        } break;
      }

    // down
    case 4:
    case 14:
    case 34: {
        if (fp.row < 2) {
          newFp.row++;
        } break;
      }
  }
  
  switch (btn) {

    // left
    case 1:
    case 12:
    case 14: {
        if(l2r) {
            // moving left2right ... can't move past endzone
            if (fp.col > 2) {
              newFp.col--;
            } break;
        } else {
            // moving right2left ... if not at endzone keep moving
            if (fp.col > 2) {
              newFp.col--;
            }
            // moving right2left ... at endzone wrap back to right side
            if (fp.col <= 2 ) {
              // could "lap" def and get tackled on other side
              fp.col = 11;  
              gs->scoreKeeper->updateFieldPosition(1);
              ranIntoDef(&fp);
              return true;
            }
            
            break;          
        }
      }

    // right
    case 3:
    case 32:
    case 34: {
        if(l2r) {
            //moving left to right ... if not at endzone keep going
            if (fp.col < 13) {
              newFp.col++;
            }
            //moving left2right ... at endzone wrap to left side
            if (fp.col >= 13) {
              // could "lap" def and get tackled on other side
              fp.col = 4;  
              gs->scoreKeeper->updateFieldPosition(1);
              ranIntoDef(&fp);
              return true;
            }
            
            break;
        } else {
            // moving right2left ... can't move past endzone
            if (fp.col < 13) {
              newFp.col++;
            } break;
        }
      }
  }

  //int ranIntoDef = gs->defenseTeam[0]->hitDefense(&newFp);

  if ( ((newFp.row != fp.row) || (newFp.col != fp.col)) && !ranIntoDef(&newFp) ) {
    if(l2r) {
       gs->scoreKeeper->updateFieldPosition(newFp.col - fp.col);
    } else {
       gs->scoreKeeper->updateFieldPosition(fp.col - newFp.col);
    }
    fp.row = newFp.row;
    fp.col = newFp.col;
    return true;
  }
  return false;

}


bool Player::ranIntoDef(displayPosition* lfp) {
  for (int i = 0; i < 5; i++)
  {
    if ( (lfp->row == gs->defenseTeam[i]->fp.row) && (lfp->col == gs->defenseTeam[i]->fp.col) )
    {
      // Tell the scorekeeper
      gs->scoreKeeper->tackle(i);
      return true;
    }
  }
  return false;
}
