
/*
 * On volca keys
 * 
 * Controls 
 *  cutoff from velocity
 *  decay  from modulation (cc:1) 
 * 
 * MIDI IN from USB and DIN
 * MIDI OUT to DIN
 */

#include <MIDI.h>

#include <midi_UsbTransport.h>

#ifndef MIN
#define MIN(a,b) ( (a) > (b) ? (b) : (a) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) < (b) ? (b) : (a) )
#endif


static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;

MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, uMIDI);

MIDI_CREATE_DEFAULT_INSTANCE();


void handleControlChange(byte channel, byte number, byte value)
{
  switch (number) {
    case 1 : // mod
      MIDI.sendControlChange(50, MAX(value - 4, 0), channel);
      break;
   }
}

void handleNoteOn(byte inChannel, byte inNumber, byte inVelocity)
{
  if (inVelocity == 0) {
    MIDI.sendNoteOn(inNumber, inVelocity, inChannel);
  } else {
    MIDI.sendNoteOn(inNumber, 127, inChannel);
    MIDI.sendControlChange(44, inVelocity, inChannel);
  }
}

void handleNoteOff(byte inChannel, byte inNumber, byte inVelocity)
{
  MIDI.sendNoteOff(inNumber, 127, inChannel);
} 

void setup() {
  uMIDI.begin(MIDI_CHANNEL_OMNI);
  uMIDI.setHandleNoteOn(handleNoteOn);
  uMIDI.setHandleNoteOff(handleNoteOff);
  uMIDI.setHandleControlChange(handleControlChange);

  uMIDI.turnThruOff();

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);

  MIDI.turnThruOff();

}

void loop() {
    uMIDI.read();
    MIDI.read();
}
