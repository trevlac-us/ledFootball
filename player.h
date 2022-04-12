#ifndef player_h
#define player_h
#include "Arduino.h"
#include "globalStash.h"
#include "controller.h"
#include "defense.h"
#include "scoreKeeper.h"
class Player {

  private:
    GlobalStash* gs;
    bool ranIntoDef(displayPosition* lfp);
    bool checkMoveWorker(byte btn);

  public:
    displayPosition fp;

    void init(GlobalStash* globalStash);
    bool checkMove();
    bool checkMoveJoyStick();
    void setPosition(byte startRow, byte startCol);

};
#endif
