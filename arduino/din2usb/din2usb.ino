/*
 * Sends notes and CC from DIN to USB
 */

#include <MIDI.h>
#include <MIDIUSB.h>

/*
#include <midi_UsbTransport.h>

static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;

MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, MIDIUSB);
*/

MIDI_CREATE_DEFAULT_INSTANCE();

//
// send midi through usb using MIDIUSB library
//
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}


// callback for din midi 
void handleNoteOn(byte inChannel, byte inNumber, byte inVelocity)
{
  noteOn(inChannel, inNumber, inVelocity);
  digitalWrite(13, HIGH);
  MidiUSB.flush();
}
void handleNoteOff(byte inChannel, byte inNumber, byte inVelocity)
{
  noteOff(inChannel, inNumber, inVelocity);
  digitalWrite(13, LOW);
  MidiUSB.flush();
}

void setup() {
    MIDI.begin();
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);

//    MIDIUSB.begin();

    pinMode(13, OUTPUT);
    
}

void loop() {
    MIDI.read();
}
