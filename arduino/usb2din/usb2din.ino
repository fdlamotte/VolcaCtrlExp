/*
 * Send midi events from USB to DIN
 */ 

#include "MIDIUSB.h"

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
//  Serial.begin(115200);
  Serial1.begin(31250);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void mNoteOn(int pitch, int velocity) {
  Serial1.write(0x90);
  Serial1.write(pitch);
  Serial1.write(velocity);
}

void mNoteOff(int pitch, int velocity) {
  Serial1.write(0x80);
  Serial1.write(pitch);
  Serial1.write(velocity);
}

void loop() {
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    switch (rx.byte1 & 0xF0) {
      case 0x90: // Note on
        mNote0n (rx.byte2, rx.byte3);
        break;
      case 0x80: // Note off
        mNoteOff (rx.byte2, rx.byte3);
        break;
    }
    
  } while (rx.header != 0);
}
