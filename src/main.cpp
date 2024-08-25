#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// Smooth buttons and potentiometers, see the libraries for examples on how to use them.
#include <Bounce2.h>
#include <ResponsiveAnalogRead.h>

const int HallAMax = 529;
const int HallAMin = 514;
const int HallBMax = 533;
const int HallBMin = 524;
const int HallAvMax = 1057;
const int HallAvMin = 1035;

// USB MIDI object
Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!");

  TinyUSBDevice.setManufacturerDescriptor("MadsKjeldgaard");
  TinyUSBDevice.setProductDescriptor("Pico Blinkity Blinky");

  usbMidi.begin();
  MIDI.begin();

  pinMode(26, INPUT);
  pinMode(27, INPUT);
  pinMode(22, OUTPUT);
  digitalWrite(22, 1);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
}

int MapToMidi(int input, int inputMin, int inputMax) {
  return min(127, max(0, map(input, inputMin, inputMax, 0, 127)));
}

int LastSentMidiA = -255;
int LastSentMidiB = -255;
int MidiA, MidiB;



bool WaitingForDrop = false;
bool BReachedMax = false;
void Mode1(){
	int analogA = analogRead(A0);
	int analogB = analogRead(A1);
  
    if(!WaitingForDrop){
		MidiA = MapToMidi(analogA, HallAMin, HallAMax);
		if (MidiA != LastSentMidiA) {
			MIDI.send(midi::ControlChange, 1, MidiA, 12U);
			LastSentMidiA = MidiA;
				if(LastSentMidiA > 120){
					WaitingForDrop = true;
				}
		}
	}
	else{
		MidiB = MapToMidi(analogB, HallBMin, HallBMax);
		if (MidiB != LastSentMidiB) {
			if(MidiB > 120){
				BReachedMax = true;
			}
			if(BReachedMax == true){
				MIDI.send(midi::ControlChange, 1, MidiB, 12U);
				LastSentMidiB = MidiB;
				if(LastSentMidiB < 5){
				BReachedMax = false;
				WaitingForDrop = false;
				}
			}		
		}
	}

	Serial.print("\nAnalog A value: ");
	Serial.print(analogA);
	Serial.print("\nAnalog B value: ");
	Serial.print(analogB);
}

void Mode0(){
  int analogA = analogRead(A0);
  int analogB = analogRead(A1);

  MidiA = MapToMidi(analogA, HallAMin, HallAMax);
  if (MidiA != LastSentMidiA) {
    MIDI.send(midi::ControlChange, 1, MidiA, 12U);
    LastSentMidiA = MidiA;
  }

  MidiB = MapToMidi(analogB, HallBMin, HallBMax);
  if (MidiB != LastSentMidiB) {
    MIDI.send(midi::ControlChange, 1, MidiB, 14U);
    LastSentMidiB = MidiB;
  }

  Serial.print("\nAnalog A value: ");
  Serial.print(analogA);
  Serial.print("\nAnalog B value: ");
  Serial.print(analogB);
}

auto pastLEDState = true;
int Mode = 1;
void loop() {
  //MIDI.read();

  // Blink the LED
  digitalWrite(LED_BUILTIN, static_cast<int>(pastLEDState));
  
  switch(Mode){
	case 0:
	Mode0();
	break;
	case 1:
	Mode1();
	break;
  }

}
