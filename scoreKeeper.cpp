#include "scoreKeeper.h"

void ScoreKeeper::init(GlobalStash* aGS) {
  gs = aGS;
  timeLeft = (gs->shortGame.dataValue[gs->shortGame.currentIndex])?75:150;
  quarter = 1;
  lastTimeUpdateMS = 0;
  clockStopped = 1;
  statusChanged = 0;
  fieldPosition = 20;
  statusDown = 1;
  yardsToGo = 10;
  oldFieldPosition = 20;
  bHomeHasBall = 1;
}

int ScoreKeeper::getHomeScore() {  return homeScore; }
int ScoreKeeper::getAwayScore() {  return awayScore; }
bool ScoreKeeper::kickIsFieldGoal() {  return bKickIsFieldGoal; }
bool ScoreKeeper::fieldGoalIsGood() {  return bFieldGoalIsGood; }

void ScoreKeeper::testKick() {
  statusDown=4;
  Serial.println(fieldPosition);
  checkKick();
}

bool ScoreKeeper::checkKick() {
  if(getStatusDown() == 4) {

    // always punt if not to 45 on other side ...
    if((fieldPosition < 55)
        // or punt  75% of time if not to 35 ... else fg attempt
        || ( (fieldPosition < 65) && (random(1,101)<=75) ) ) {
      bKickIsFieldGoal=0;
      bFieldGoalIsGood=0;

      // punt field position change
      fieldPosition = fieldPosition + random(10, 60);
      fieldPosition = (fieldPosition>99)?20:100 - fieldPosition;
    
    } else {
      bKickIsFieldGoal=1;

      // FG is good 40% long or 85% short
      if( ( (fieldPosition < 65) && (random(1,101) <=40))
          || (fieldPosition >= 65) && (random(1,101) <=85)) {
              bFieldGoalIsGood=1;
              fieldPosition = 20;

              if(bHomeHasBall) {
                homeScore = homeScore + 3;
              }else {
                awayScore = awayScore +3;
              }
      
      } else {
          // missed Field Goal
          bFieldGoalIsGood=0;
          fieldPosition = 100 - fieldPosition;

      }
    }

    // -- change sides
    statusDown = 1;
    yardsToGo = 10;
    bHomeHasBall = !bHomeHasBall;

    gs->gameMode = 8;
  
  }
}

void ScoreKeeper::updateFieldPosition(int change) {
  fieldPosition = fieldPosition + change ;

  // Touch down
  if(fieldPosition >99) {

      if(bHomeHasBall) {
        homeScore = homeScore + 7;
      }else {
        awayScore = awayScore +7;
      }

      // -- change sides
      statusDown = 1;
      yardsToGo = 10;
      fieldPosition = 100 - fieldPosition;
      bHomeHasBall = !bHomeHasBall;

    
      gs->gameMode = 42; //touch down
  }
  statusChanged = true;
}

int ScoreKeeper::getFieldPosition() {
      if(homeHasBall()) {
        if (ballIsOnHomeSide()) {
          return  fieldPosition;
        } else {
          return 100 - fieldPosition;
        }
      // visitors have ball
      } else {
        if (ballIsOnHomeSide()) {
          return 100 - fieldPosition;
        } else {
          return fieldPosition;
        }
      }
}

bool ScoreKeeper::ballIsOnHomeSide() {
    if(homeHasBall() && fieldPosition <=50) return true;
    if(!homeHasBall() && fieldPosition >50) return true;
    return false;
}

int ScoreKeeper::getStatusDown() {
      return statusDown;
}

int ScoreKeeper::getYardsToGo() {
      return yardsToGo;
}

bool ScoreKeeper::homeHasBall() {
      return bHomeHasBall;
}

void ScoreKeeper:: tackle(byte defMan){
  gs->defManTackle = defMan;
  gs->audio->playSoundTweet();
  gs->gameMode = 5;

  // update down & toGO
  int yardsMoved = fieldPosition - oldFieldPosition;

  // 1st down
  if(yardsMoved > 9) {
      statusDown = 1;
      yardsToGo = 10;
      gs->gameMode = 51;
      
  } else {
    statusDown++;
    yardsToGo = yardsToGo - yardsMoved;
  }

  // change of sides logic goes here
  if(statusDown > 4) {
      // 2 tweets
      gs->audio->playSoundTweetX2();

      statusDown = 1;
      yardsToGo = 10;
      fieldPosition = 100 - fieldPosition;
      bHomeHasBall = !bHomeHasBall;
  }

  oldFieldPosition = fieldPosition;
  statusChanged = true;
  
}

int ScoreKeeper::getQuarter() { return quarter;}
void ScoreKeeper::updateQuarter() {  
   timeLeft = (gs->shortGame.dataValue[gs->shortGame.currentIndex])?75:150;
   quarter++;

  if(quarter==2) {
      // change posession
      statusDown = 1;
      yardsToGo = 10;
      fieldPosition = 20;
      bHomeHasBall = !bHomeHasBall;
  }
   
}



void ScoreKeeper::updateClock() {
  unsigned long currentMS = millis();

  if (clockStopped) {
    lastTimeUpdateMS = currentMS;
    clockStopped = false;
  } else {
    if (currentMS > lastTimeUpdateMS + 1000) {
      timeLeft = timeLeft - 1;

      lastTimeUpdateMS = currentMS;
      scoreChanged = true; // time is on the score screen

      if(timeLeft = 0) {
        gs->gameMode = 9;
        clockStopped = true;
        gs->audio->playSoundTweetX2();
      } else {
         gs->audio->playSoundTick();
        
      }
      
    }
  }

}

bool ScoreKeeper::checkStatusChange() {
  if (!statusChanged) return false;
  statusChanged = false;
  return true;
}

bool ScoreKeeper::checkScoreChange() {
  if (!scoreChanged) return false;
  scoreChanged = false;
  return true;
}

int ScoreKeeper::getClock() {
  return timeLeft;
}
void ScoreKeeper::stopClock() {
  clockStopped = true;
}
