#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// Smooth buttons and potentiometers, see the libraries for examples on how to use them.
//#include <Bounce2.h>
//#include <ResponsiveAnalogRead.h>

#define SERIALPRINT 0

const int HallAMax = 2100;
const int HallAMin = 2050;
const int HallBMax = 2800;
const int HallBMin = 2100;

// USB MIDI object
Adafruit_USBD_MIDI usbMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbMidi, MIDI);

void setup() {
#if SERIALPRINT
  Serial.begin(115200);
#endif

  TinyUSBDevice.setManufacturerDescriptor("Christopher Lee");
  TinyUSBDevice.setProductDescriptor("Fidget 3000 by Chris");

  usbMidi.begin();
  MIDI.begin();

  pinMode(A2, INPUT);
  pinMode(A1, INPUT);

  pinMode(10, INPUT_PULLDOWN);
  pinMode(11, INPUT_PULLDOWN);

  pinMode(22, OUTPUT_4MA);
  digitalWrite(22, HIGH);

  pinMode(21, OUTPUT_4MA);
  digitalWrite(21, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  analogReadResolution(12);
}

void loop1(){
	digitalWrite(LED_BUILTIN, HIGH);
	delay(250);
	digitalWrite(LED_BUILTIN, LOW);
	delay(250);
}

int MapToMidi(int input, int inputMin, int inputMax) {
  return min(127, max(0, map(input, inputMin, inputMax, 0, 127)));
}

int LastSentMidiA = -255;
int LastSentMidiB = -255;
int MidiA, MidiB;
int analogA;
int analogB;



bool WaitingForDrop = false;
bool BReachedMax = false;
void PickupAndDropMode(){
    if(!WaitingForDrop){
#if SERIALPRINT
		Serial.print("   Rising, waiting for drop...");
#endif
		if (MidiA != LastSentMidiA) {
			MIDI.send(midi::ControlChange, 1, MidiA, 12U);
			LastSentMidiA = MidiA;
				if(LastSentMidiA > 120){
					WaitingForDrop = true;
				}
		}
	}
	else{
#if SERIALPRINT
		Serial.print("   dropping, waiting for rise...");
#endif
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
}

void SendTogether(){ //sends both pickups on the same channel
  if (MidiA != LastSentMidiA) {
    MIDI.send(midi::ControlChange, 1, MidiA, 12U);
    LastSentMidiA = MidiA;
  }

  if (MidiB != LastSentMidiB) {
    MIDI.send(midi::ControlChange, 1, MidiB, 12U);
    LastSentMidiB = MidiB;
  }
}

void SendSeperately(){ //sends each pickup as its own channel
  if (MidiA != LastSentMidiA) {
    MIDI.send(midi::ControlChange, 1, MidiA, 12U);
    LastSentMidiA = MidiA;
  }

  if (MidiB != LastSentMidiB) {
    MIDI.send(midi::ControlChange, 1, MidiB, 14U);
    LastSentMidiB = MidiB;
  }
}

auto pastLEDState = true;
int Mode = -1;
void loop() {
	if (BOOTSEL) {
		rp2040.rebootToBootloader();
	}

  analogA = analogRead(A2);
  analogB = analogRead(A1);
  MidiA = MapToMidi(analogA, HallAMin, HallAMax);
  MidiB = MapToMidi(analogB, HallBMin, HallBMax);
#if SERIALPRINT
  Serial.print("\nAnalog A value: ");
  Serial.print(analogA);
  Serial.print("\nAnalog B value: ");
  Serial.print(analogB);
  Serial.print("\nMapped A value: ");
  Serial.print(MidiA);
  Serial.print("\nMapped B value: ");
  Serial.print(MidiB);
  Serial.print("\nLast sent channel A: ");
  Serial.print(LastSentMidiA);
  Serial.print("\nLast sent channel B: ");
  Serial.print(LastSentMidiB);
#endif
  if(HIGH == digitalRead(10)){
	Mode = 0;
  }
  else if(HIGH == digitalRead(11)){
	Mode = 1;
  }
  else{
	Mode = 2;
  }
  
  switch(Mode){
	case 0:
	PickupAndDropMode();
#if SERIALPRINT
	Serial.println("\nPickupAndDropMode");
#endif
	break;
	case 1:
	SendTogether();
#if SERIALPRINT
	Serial.println("\nSendTogether");
#endif
	break;
	case 2:
	SendSeperately();
#if SERIALPRINT
	Serial.println("\nSendSeperately");
#endif
	break;
  }
  
#if SERIALPRINT
  delay(20);
#else
  delay(5);
#endif
}
