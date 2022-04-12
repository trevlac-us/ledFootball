#ifndef scoreKeeper_h
#define scoreKeeper_h
#include "Arduino.h"
#include "globalStash.h"
#include "audio.h"

class ScoreKeeper {

  private:
    GlobalStash* gs;
    int timeLeft;
    unsigned long lastTimeUpdateMS;
    bool clockStopped;
    bool statusChanged;
    bool scoreChanged;
    int fieldPosition;
    int statusDown;
    int yardsToGo;
    int oldFieldPosition;
    bool bHomeHasBall;
    int homeScore=0;
    int awayScore=0;
    bool bKickIsFieldGoal=0;
    bool bFieldGoalIsGood=0;
    int quarter=1;

  public:
 
    void init(GlobalStash* globalStash);
    void updateClock();
    bool checkStatusChange();
    bool checkScoreChange();
    int getClock();
    void stopClock();
    int getFieldPosition();
//    bool checkTouchDown();
    void updateFieldPosition(int c);
    void tackle(byte defMan);
    int getStatusDown();
    int getYardsToGo();
    bool homeHasBall();
    bool ballIsOnHomeSide();
    int getHomeScore();
    int getAwayScore();
    bool checkKick();
    bool kickIsFieldGoal();
    bool fieldGoalIsGood();
    void testKick();
    int getQuarter();
    void updateQuarter();
};
#endif
