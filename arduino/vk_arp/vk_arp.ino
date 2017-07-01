/*
 * Simple arpegiator
 * 
 * Used with Volca keys
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

void setup() {
  uMIDI.begin(MIDI_CHANNEL_OMNI);
  uMIDI.setHandleNoteOn(handleNoteOn);
  uMIDI.setHandleNoteOff(handleNoteOff);

  uMIDI.turnThruOff();

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

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
      MIDI.sendNoteOff(cur_note, 0, 1);
    } else {
      timer0 = 0;
    }

    if (pressed_keys) {
      arp_on = 1;
      cur_key = (cur_key + 1) % MIN(pressed_keys, 10);
      cur_note = keys[cur_key];
      MIDI.sendNoteOn(cur_note, 127, 1);
    } else {
      arp_on = 0;
    }

    // blink
    int ledPin = digitalRead(led);
    digitalWrite(led, !ledPin);
  }
}
