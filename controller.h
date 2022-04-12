#ifndef controller_h
#define controller_h
#include "Arduino.h"

class Controller {

  public:
    init();
    byte getJoyStick ();
    byte getButtonPressed ();
    byte getButtonHeld ();
    byte getButtonReleased ();
    bool getStatusPressed();
    bool getScorePressed();

  private:

    // -- Buttons
    const byte buttonDelayMS = 100;
    // map voltages (position in array 0,1,2,3,4,5) to
    //   what code uses 1=left,2=up,3=right,4=down,0=none,5=joy
    const byte buttonIOmap[6] = {2, 3, 4, 1, 5, 0};
    
    // map non linear ranges ... ends only get 1/2 range
    const byte nonLinearVoltageMap[10] = {0,1,1,2,2,3,3,4,4,5};
    
    const int buttonsPin = A0;
    const int statusPin = A3;
    const int scorePin = A4;

    unsigned long lastStBtnCheckMS = 0;
    unsigned long lastScBtnCheckMS = 0;
    unsigned long lastBtnCheckMS = 0;

    byte buttonPressed = 0; // 0=none, then 1-5
    byte lastButtonPressed = 0;
    byte buttonHeld = 0;  // 0 or 1
    byte statusHeld = 0;  // 0 or 1
    byte scoreHeld = 0;  // 0 or 1

    // -- Joystick
    const byte joyStickDelayMS = 250;
    const int joyStickXPin = A1;
    const int joyStickYPin = A2;
    const byte joyStickIOmap[3][3] = {{14, 4, 34}
      , {1, 0, 3}
      , {12, 2, 32}
    };

    byte joyStickDirection = 0; 
    // Starting 12 oClock clockwise ... try to match buttons
    // 2=up, 32=right-up, 3=right, 34=right-down
    // 4=down, 14=left-down, 1=left, 12=left-up
    // 0=center


    unsigned long lastJoyStickCheckMS = 0;

    void pReadJoyStick();
    void pReadButton();

};
#endif
