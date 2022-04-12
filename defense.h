#ifndef defense_h
#define defense_h
#include "Arduino.h"
#include "globalStash.h"
#include "player.h"
#include "scoreKeeper.h"

class Defense {

  private:
    GlobalStash* gs;
    bool hitPlayer(displayPosition* fp);
    bool hitDefense(displayPosition* fp);

  public:
    displayPosition fp;
    byte manNumber;
    void init(GlobalStash* globalStash,byte manNum);
    bool checkMove();
    void setPosition(byte startRow, byte startCol);


};
#endif
