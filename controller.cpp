
#include "controller.h"
/*****************************************************
   Class deals with the game controller / buttons

 *****************************************************/

bool Controller::getStatusPressed() {

    if ((millis() > lastStBtnCheckMS + buttonDelayMS)) {
      lastStBtnCheckMS = millis();
      
      // 0=holding status button / pin low
      if(!digitalRead(statusPin) && !statusHeld) {
        statusHeld = 1;
        return true;
      }else {
        statusHeld = 0;
      }
    }

  return false;
}

bool Controller::getScorePressed() {
    if ((millis() > lastScBtnCheckMS + buttonDelayMS)) {
      lastScBtnCheckMS = millis();
      
      // 0=holding score button / pin low
      if(!digitalRead(scorePin) && !scoreHeld) {
        scoreHeld = 1;
        return true;
      }else {
        scoreHeld = 0;
      }      
   }
   return false;
 }


void Controller::pReadJoyStick() {
  if ((millis() > lastJoyStickCheckMS + joyStickDelayMS)) {

    // grab this in a var so we can debug
    int valX = analogRead(joyStickXPin);
    int valY = analogRead(joyStickYPin);
    joyStickDirection = joyStickIOmap[map(valX, 0, 1025, 0, 3)]
                        [map(valY, 0, 1025, 0, 3)];
    lastJoyStickCheckMS = millis();

    // set zero to enforce delay
  } else {
    joyStickDirection = 0;
  }
}


void Controller::pReadButton() {
  if ((millis() > lastBtnCheckMS + buttonDelayMS)) {
    lastButtonPressed = buttonPressed;

    // grab this in a var so we can debug
    int val = analogRead(buttonsPin);

    // map voltage input of 0-1023 to non linear voltage cells (0-9) and get values of 0-5 
    //    where 0 and 5 are only 1/2 as wide as 1,2,3,4 in the nonLinearVoltageMap array
    // then map voltages 0-5 to buttons 0-5 
    //    where 5 for example means no button pushed ... aka button zero (flase)
    // map in and out max ranges need to be + 1 as map.outMax is excluded due to int math
    buttonPressed = buttonIOmap[map(nonLinearVoltageMap[map(val,0,1025,0,10)],0,6,0,6)];
    
    lastBtnCheckMS = millis();
  }
}

Controller::init() {
  pinMode(buttonsPin, INPUT);
  pinMode(joyStickXPin, INPUT);
  pinMode(joyStickYPin, INPUT);
  pinMode(statusPin, INPUT);
  pinMode(scorePin, INPUT);  
}


//************************************************
//************************************************
byte Controller::getJoyStick () {
  pReadJoyStick();
  return joyStickDirection;
}


//************************************************
// Go from not pressing to pressing
// and button not marked as held
// mark as held if true so rapid calls here don't keep giving a true
//   before pReadButton gets around to updating
// don't mark as !held as we could be holding !pressing
//************************************************
byte Controller::getButtonPressed () {


  // see if button was released
  if (lastButtonPressed != 0 && buttonPressed == 0 && buttonHeld) {
    buttonHeld = 0;
  }
  pReadButton();

  if (lastButtonPressed == 0 && buttonPressed != 0 && !buttonHeld) {
    buttonHeld = 1;
    return buttonPressed;
  }
  return 0;
}

//************************************************
// Must had been pressing and still pressing
// mark as held if true, mark not held if not true
//************************************************
byte Controller::getButtonHeld () {
  pReadButton();
  if (lastButtonPressed != 0 && buttonPressed != 0) {
    buttonHeld = 1;
    return buttonPressed;
  }
  buttonHeld = 0;
  return 0;
}

//************************************************
// Must had been pressing and now not pressing & held
// mark as !held if true so rapid calls here don't keep giving release
//   before pReadButton gets around to updating
// can get odd results if press 2 buttons aka press 1 release other
//************************************************
byte Controller::getButtonReleased () {

  if (lastButtonPressed == 0 && buttonPressed != 0 && !buttonHeld) {
    buttonHeld = 1;
  }
  pReadButton();

  if (lastButtonPressed != 0 && buttonPressed == 0 && buttonHeld) {
    buttonHeld = 0;
    return lastButtonPressed;
  }
  return 0;
}
