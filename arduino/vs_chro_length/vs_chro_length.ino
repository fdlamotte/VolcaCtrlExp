/*
 * MIDI IN : DIN
 * MIDI OUT : DIN
 *
 * Translates notes to volca sample pitches (chromatic play)
 *  => monophonic
 *
 * Only sends note on on first key, others change pitch on current channel
 * 
 * PC : changes channel
 * Modulation (CC:1) : changes sample length 
 */
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

#define LED 13

#ifndef MIN
#define MIN(a,b) ( (a) > (b) ? (b) : (a) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) < (b) ? (b) : (a) )
#endif

//Note: Midi CC value 64 = Speed value 0
const int vSpeedValue[] = { //Pitches (speed) for each note
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19,  20,  21,  22,  23,  24,  25,  
26, 27, 28, 29, 30, 32, 34, 37, 40, 43, 45, 48, 51, 53, 56, 59, 61, 64, 67, 69, 72, 75, 78, 80, 83, 85, 88, 91, 93, 96, 97, 99, 100, 102, 103, 104, 106, 107, 108, 109, 111, 112
};

byte base_chan = 1;
byte notes = 0;


void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  MIDI.sendControlChange(43, vSpeedValue[pitch], base_chan);

  if (notes == 0) {
    MIDI.sendNoteOn(0, velocity, base_chan);
    digitalWrite(LED, HIGH);
  }

  notes ++;
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (--notes < 0) { 
    digitalWrite(LED, LOW);
    notes = 0;
  }
}

void handleControlChange(byte channel, byte number, byte value)
{
  switch (number) {
    case 1 : // mod
      //MIDI.sendControlChange(43, MIN((value * 127) / 99, 127), base_chan);
      MIDI.sendControlChange(40, MAX(value - 4, 0), base_chan);
      break;
  }
}


void handleProgramChange(byte channel, byte number)
{
  base_chan = number % 10 + 1;  
}

void setup()
{
    pinMode(LED, OUTPUT);

    MIDI.turnThruOff();
    
    MIDI.setHandleNoteOn(handleNoteOn);  
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandleControlChange(handleControlChange);
    MIDI.setHandleProgramChange(handleProgramChange);
        
    MIDI.begin(MIDI_CHANNEL_OMNI);

    MIDI.turnThruOff();

}

void loop()
{
    MIDI.read();
}
