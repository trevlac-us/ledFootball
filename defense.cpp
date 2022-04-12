/*****************************************************
   Class of a defenseMan
   basically keeps track of position and how to move
 *****************************************************/
#include "defense.h"


/******************************************************
   Get pointer to Globals
   Set initial position
******************************************************/
void Defense::init(GlobalStash* globalStash,byte mn) {
  gs = globalStash;
  manNumber = mn;
}

void Defense::setPosition(byte startRow, byte startCol) {
  fp.row = startRow;
  fp.col = startCol;
}


//**********************************************
//**********************************************
bool Defense::checkMove() {

  displayPosition newFp;
  newFp.row = fp.row;
  newFp.col = fp.col;

  displayPosition playerFp;
  playerFp.row = gs->player->fp.row;
  playerFp.col = gs->player->fp.col;
  

  // want to move towards player on a delayed basis
  bool rnd = random(0,2);
    //if(fp.row==playerFp.row) {
    if(rnd) {
      // move column
      if(playerFp.col < fp.col) {
        newFp.col--;      
      } else {
        newFp.col++;
      }
    } else {
      // move row
      if(playerFp.row < fp.row) {
        newFp.row--;      
      } else {
        newFp.row++;
      }
    }

    
  // Did we move ?
  // row or col changed 
  // & didn't hit def
  // & didn't hit player
  // & didn't go off sides
  if ( ((newFp.row != fp.row) || (newFp.col != fp.col)) 
       && !hitDefense(&newFp) 
       && !hitPlayer(&newFp)
       && newFp.row <=2
       && newFp.row >=0
     ) 
  {
    fp.row = newFp.row;
    fp.col = newFp.col;
    return true;
  }
  return false;
}  

bool Defense::hitPlayer(displayPosition* lfp) {
  if ( (lfp->row == gs->player->fp.row) && (lfp->col == gs->player->fp.col)) {
    // Tell the scorekeeper
    gs->scoreKeeper->tackle(manNumber);
    return true;
  } else {
    return false;
  }
}

bool Defense::hitDefense(displayPosition* lfp) {
  for (int i = 0; i < 5; i++)
  {
    if ( (lfp->row == gs->defenseTeam[i]->fp.row) && (lfp->col == gs->defenseTeam[i]->fp.col) )
    {
      return true;
    }
  }
  return false;
}
