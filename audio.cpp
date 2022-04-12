#include "audio.h"

//*****************************************
//** Initialize Static Vars
//*****************************************
  unsigned char Audio::audioVolume = 2;

  int Audio::noteListLength=0;
  unsigned char const* Audio::pNoteFreqList=0;
  unsigned int const* Audio::pNoteDurList=0;

  volatile int Audio::currentNotePosition=0;
  volatile unsigned int Audio::currentNoteFreq=0;
  volatile unsigned int Audio::currentNoteDur=0;
  volatile unsigned int Audio::noteHalfFreqCounter = 0;
  volatile unsigned int Audio::noteDurationCounter = 0;

  int Audio::sampleListLength=0;
  unsigned char const* Audio::pSampleList= 0;
  volatile int Audio::currentSamplePosition= 0;
  volatile unsigned int Audio::sampleDurationCounter= 0;
  volatile unsigned char Audio::repeatTimes=0;



// ==================================================================================
//  playSong()
//
//  Setup Timer0 in fastPWM which will Toggle Pin 3 between the value in OCR2B & Zero
//  Net effect is to PWM of duty cycle in OCR2B to control volume at CPU Speed /256 = 62500HZ
//  Also, on match fire Interupt where we change OCR2B between volume and zero to output
//  a longer square wave at the note freq ... doing this for the note duration
// ==================================================================================
void Audio::playSong(unsigned char const *noteFreq, unsigned int const *noteDuration, int songLength){
  stopPlay();
  
  //********** Initialize Static Vars used in songCompareMatch_Interupt() ********
  // *** Arrays we are going to loop thru
  noteListLength = songLength;
  pNoteFreqList = noteFreq;
  pNoteDurList = noteDuration;

  // *** Counters to track the note we are on, the notes freq from max to zero square wave, and len playing so far
  currentNotePosition = 0;
  noteHalfFreqCounter = 0;
  noteDurationCounter = 0;

  //*** Get 1st values out of PGMMem to Ram ?for speed?
  currentNoteFreq = pgm_read_byte_near(pNoteFreqList + currentNotePosition)<<2;
  currentNoteDur = pgm_read_word_near(pNoteDurList + currentNotePosition);

  // Compare match ISR calls this function pointer
  Audio::compareMatchInteruptPtr = &Audio::songCompareMatch_Interupt;  

  startPlay();

}

// ==================================================================================
// ==================================================================================
void Audio::playSound(unsigned char const *samples, int samplesLength, unsigned char numTimes){
  stopPlay();

  //********** Initialize Static Vars used in soundCompareMatch_Interupt() ********
  // *** Array we are going to loop thru
  sampleListLength = samplesLength;
  pSampleList = samples;
  repeatTimes = numTimes;

  // *** Counters to track the sample we are on, and len playing so far
  currentSamplePosition = 0;
  sampleDurationCounter = 0;

  // Compare match ISR calls this function pointer
  Audio::compareMatchInteruptPtr = &Audio::soundCompareMatch_Interupt;  

  startPlay();

}

// ==================================================================================
// ==================================================================================
void Audio::startPlay() {


    //OC2B is Arduino Pin 3
    pinMode(3, OUTPUT);
  
    // Timer/Counter Control Register(s) Configuration
    TCCR2A = 0b00100011;
    // Bit#
    //  7,6,5,4 - Compare output mode - OC2A [00] Disconnected, OC2B [10] Non-Inverting (zero on match Set to OCR2B on bottom)
    //  3,2     - Not used set to [00]
    //  1,0     - WGM01,WGM00: Waveform Gen Mode [11] = fastPWM mode 3 ... count up to 256 (vs mode7 which counts to value in OCR2A)
    
    TCCR2B = 0b00000001;
    // Bit#
    //  7,6   - For non-PWM ... set to [00] for PWM
    //  5,4   - Not used ... set to [00]
    //  3     - WGM02: used to set Waveform Gen Mode along with bits 1,0 in TCCR2A.  Set to 0 for fastPWM Mode3
    //  2,1,0 - Clock Source / Divider CS02,CS01,CS00 [001] = no prescale
    
    // Output Compare Register 
    // Going to filp between 255(max vol) and zero ... This will give "note Frequency" square wave
    OCR2B = 0; //start with zero so we don't need to keep checking "rest" freqs
  
    // Timer Interupt Mask - Config Interupts On
    // OCIE2B (Bit2) - Compare/Match B Enabled (Pin 3)
    TIMSK2 = 0b00000100;
    // Bit#
    //  7-3 - Not used set to [00000]
    //  2   - Compare Match B [1] OCR2B value
    //  1   - Compare Match A [0] OCR2A value
    //  0   - Overflow
}



// ==================================================================================
//    stopPlay()
//
//      See startPlay() for more info on what we are "stopping"
// ==================================================================================
static void Audio::stopPlay() {
    // Turn off interrupt bit 2
    //TIMSK2 = 0b00000000;
    bitClear(TIMSK2,2); 
  
    // Disable Timer2 completely by setting Clock/Prescale to
    //    "No clock source (Timer/Counter stopped)" ...  bits 2-0; [000]
    TCCR2B = 0b00000000;
    
    //OC2B is Arduino Pin 3 ... set to low
    digitalWrite(3, 0);
}


//************************************
//  Static Methods for ISR to call
//************************************

// ==================================================================================
//    songCompareMatch_Interupt()
// ==================================================================================
static void Audio::songCompareMatch_Interupt() {

  // ** We are playing the note
  if(++noteDurationCounter < currentNoteDur) {

    //skip freq toggle if it is a "rest" = 0 freq
    if(currentNoteFreq > 0) {
      if(++noteHalfFreqCounter >= currentNoteFreq){

          //toggle between volMax and zero to get freq square wave
          OCR2B = (OCR2B==0)?(255>>audioVolume):0;
          noteHalfFreqCounter=0;
      }
    }

  // ** We need the next note
  } else {
    if(++currentNotePosition < noteListLength) {

      //** Get the note info from PGMmem and start with zero half of square wave
      currentNoteFreq = pgm_read_byte_near(pNoteFreqList + currentNotePosition)<<2;
      currentNoteDur = pgm_read_word_near(pNoteDurList + currentNotePosition);
      OCR2B = 0;

      //** Reset the counters
      noteHalfFreqCounter = 0;
      noteDurationCounter = 0;

    // ** End of Song
    } else {
      stopPlay();
    }
  }
  
}

// ==================================================================================
//    soundCompareMatch_Interupt()
// ==================================================================================
static void Audio::soundCompareMatch_Interupt() {

  // ** Fixed sample rate = 8000 samples per sec (actually 7,812.5 samples/sec)
  //    This ISR fires at CPU_Freq / 256 ... 16000000 / (256 * 8000) =~ 8 
  if(++sampleDurationCounter > 8) {

    if(++currentSamplePosition< sampleListLength) {
      OCR2B = (pgm_read_byte_near(pSampleList + currentSamplePosition)>>audioVolume);

      //** Reset the counter
      sampleDurationCounter = 0;

    // ** End of samples
    } else {
      // repeat ?
      if(--repeatTimes>0) {
          currentSamplePosition = 0;
          sampleDurationCounter = 0;
      } else {
        stopPlay();
      }
    }
  }
  
}


//**********************************************
//  This stuff is for ISR to hook back to class
//*********************************************

//**** Compile time initilization of fn Pointer ISR will call
typedef void vectorFN();
vectorFN* Audio::compareMatchInteruptPtr = &Audio::songCompareMatch_Interupt;

//**** Have ISR call method stored in Static Class function pointer
ISR(TIMER2_COMPB_vect)  { Audio::compareMatchInteruptPtr();}



// =============================================
// Main Public methods to play sounds/songs
// =============================================
void Audio::playSongCharge() {playSong(ChargeMelody, ChargeDurations, sizeof(ChargeDurations)/sizeof(ChargeDurations[0]));}
void Audio::playSongRock() {playSong(RockMelody, RockDurations, sizeof(RockDurations)/sizeof(RockDurations[0]));}
void Audio::playSongThunder() {playSong(ThunderMelody, ThunderDurations, sizeof(ThunderDurations)/sizeof(ThunderDurations[0]));}
void Audio::playSongMrTouchDown() {playSong(MrTouchDownMelody, MrTouchDownDurations, sizeof(MrTouchDownDurations)/sizeof(MrTouchDownDurations[0]));}
void Audio::playSoundTweet(){playSound(tweet,sizeof(tweet)/sizeof(tweet[0]),1);}
void Audio::playSoundTick(){playSound(tick,sizeof(tick)/sizeof(tick[0]),1);}
void Audio::playSoundTweetX2() {playSound(tweet,sizeof(tweet)/sizeof(tweet[0]),2);}
