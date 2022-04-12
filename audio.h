#ifndef audio_h
#define audio_h
#include "globalStash.h"
#include <avr/interrupt.h>
#include "Arduino.h"


class Audio {

  public:
    //*** Static pointer to interupt method ... called by ISR
    static  void (*compareMatchInteruptPtr)();

    //*** Main public Methods
    void playSongCharge();
    void playSongRock();
    void playSongThunder();
    void playSongMrTouchDown();
    void playSoundTweet();
    void playSoundTick();
    void playSoundTweetX2();
    static unsigned char audioVolume;

    

  private:


    //**** Data Used by SONG CompareMatch_Interupt()
      //*** Dealing with PGMMem song data ****
      static int noteListLength;                  //Length of song
      static unsigned char const* pNoteFreqList;  //ptr to const Array of NoteFreqs
      static unsigned int const* pNoteDurList;    //ptr to const Array of NoteDurations
      static volatile int currentNotePosition;    //counter as we move thru notes
      static volatile unsigned int currentNoteFreq;  //for speed? we pull these values
      static volatile unsigned int currentNoteDur;   //   from PGMMem into ram 
  
      static volatile unsigned int noteHalfFreqCounter;
      static volatile unsigned int noteDurationCounter;

    //**** Data Used by SOUND CompareMatch_Interupt()
      static int sampleListLength;                  //# samples in sound
      static unsigned char const* pSampleList;  //ptr to const Array of samples
      static volatile int currentSamplePosition;    //counter as we move thru samples
      static volatile unsigned int sampleDurationCounter;
      static volatile unsigned char repeatTimes;                  //repeat the sound


    //**** Private Methods
    void playSong(unsigned char const *noteFreq, unsigned int const *noteDuration, int songLength);
    void playSound(unsigned char const *samples, int samplesLength, unsigned char numTimes);
    static void startPlay();
    static void stopPlay();

    //** Interupt Handlers
    static void songCompareMatch_Interupt();
    static void soundCompareMatch_Interupt();


};

#endif
