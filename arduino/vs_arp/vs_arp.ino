/*
 * Simple arpegiator
 * 
 * Used with Volca sample
 * 
 * Uses both USB and DIN as MIDI IN 
 *      DIN as MIDI OUT
 * 
 * Plays pressed notes in order
 * 
 * Analog in A0 inputs tempo value
 * 
 * Uses a seven segment display to show the time interval between notes 
 * TODO: Convert to tempo
 * 
 * 
 */


#include <elapsedMillis.h>
#include <MIDI.h>
#include <midi_UsbTransport.h>

#include "segs.h"


#ifndef MAX
#define MAX(a,b) ( (a) < (b) ? (b) : (a) )
#endif

#ifndef MIN
#define MIN(a,b) ( (a) > (b) ? (b) : (a) )
#endif

static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;

MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, uMIDI);

MIDI_CREATE_DEFAULT_INSTANCE();

// Notes
//Note: Midi CC value 64 = Speed value 0
const int vSpeedValue[] = { //Pitches (speed) for each note
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19,  20,  21,  22,  23,  24,  25,  
26, 27, 28, 29, 30, 32, 34, 37, 40, 43, 45, 48, 51, 53, 56, 59, 61, 64, 67, 69, 72, 75, 78, 80, 83, 85, 88, 91, 93, 96, 97, 99, 100, 102, 103, 104, 106, 107, 108, 109, 111, 112
};
byte base_chan = 1;


// timer variables
int interval = 500; // in ms
elapsedMillis timer0;

int tempo_pin = A0;
int tempo_pin_value;

// LED
int led = 13;

// keys
int keys[10];
int pressed_keys = 0;
int cur_key = 0;
int cur_note = 0; // last note value to send correct noteOff 
int arp_on = 0;

void handleNoteOn(byte inChannel, byte inNumber, byte inVelocity)
{
  int i = pressed_keys;
  if (pressed_keys < 10) {
    while ((i > 0) && (keys[i-1] > inNumber)) {
      keys[i] = keys[i-1];
      i--;
    }
    keys[i] = inNumber;    
  }
  pressed_keys++;
}

void handleNoteOff(byte inChannel, byte inNumber, byte inVelocity)
{
  int i = 0;
  for (i = 0; (i < 9) && (keys[i]!=inNumber);i++);
  for (;i < 9;i++) {    
    keys[i]=keys[i+1];
  }
  pressed_keys--;
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



void setup() {
  uMIDI.begin(MIDI_CHANNEL_OMNI);
  uMIDI.setHandleNoteOn(handleNoteOn);
  uMIDI.setHandleNoteOff(handleNoteOff);
  uMIDI.setHandleControlChange(handleControlChange);
  uMIDI.setHandleProgramChange(handleProgramChange);

  uMIDI.turnThruOff();

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);
        
  MIDI.turnThruOff();

  pinMode(led, OUTPUT);

  timer0 = 0;

  segs_setup();
}

void loop() {
  uMIDI.read();
  MIDI.read();

  tempo_pin_value = analogRead(tempo_pin);
  interval = tempo_pin_value;

  displayNumber(interval);
  
  if ((!arp_on) || (!pressed_keys) || (timer0 > interval)) {

    if (arp_on) {
      timer0 -= interval; //reset the timer
      MIDI.sendNoteOff(0, 0, base_chan);
    } else {
      timer0 = 0;
    }

    if (pressed_keys) {
      arp_on = 1;
      cur_key = (cur_key + 1) % MIN(pressed_keys, 10);
      cur_note = keys[cur_key];

      MIDI.sendControlChange(43, vSpeedValue[cur_note], base_chan);

      MIDI.sendNoteOn(cur_note, 127, base_chan);
    } else {
      arp_on = 0;
    }

    // blink
    int ledPin = digitalRead(led);
    digitalWrite(led, !ledPin);
  }
}
